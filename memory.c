#include "defs.h"
#include "include/mmu.h"
#include "include/types.h"
#include "include/string.h"
#include "include/x86.h"

pde_t *kpgdir;

uint8_t *mem_start, *mem_end;
size_t mem_length;
extern uint8_t data[], end[];

struct segdesc gdt[NSEGS];

struct page_t {
    struct page_t *next;
};

struct {
  struct page_t *freelist;
} kmem;

void memory_init(void) {
    vga_console_printf("Initializing memory...\n");
    get_memory_layout();
    free_range(end, P2V(4 * 1024 * 1024));
    kpgdir = setupkvm();
    switchkvm();
    seg_init();
    free_range(P2V(4 * 1024 * 1024), P2V(mem_end));
    vga_console_printf("%d bytes available.\n", mem_length);
}

void get_memory_layout(void) {
    struct e820_table *base = (struct e820_table *)P2V(0x8000);
    if (base->nr_entries == 0) {
        panic("e820 failed");
    };
    for (size_t i = 0; i < base->nr_entries; i++) {
        if (base->entries[i].type == 1 && base->entries[i].length_low > 0x1000000) {
            mem_start = (uint8_t *)(base->entries[i].base_addr_low);
            mem_length = base->entries[i].length_low;
            mem_end = (uint8_t *)(base->entries[i].base_addr_low + base->entries[i].length_low);
            
            return;
        }
    }
}

void seg_init(void) {
    gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
    gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
    gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
    gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
    lgdt(gdt, sizeof(gdt));
}

void kfree(uint8_t *v) {
    struct page_t *p;

    if((uint32_t)v % PGSIZE || v < end || V2P(v) >= (uint32_t)mem_end)
        panic("kfree");

    memset(v, 1, PGSIZE);

    p = (struct page_t *)v;
    p->next = kmem.freelist;
    kmem.freelist = p;
}

void free_range(void *vstart, void *vend) {
    uint8_t *p;
    p = (uint8_t *)PGROUNDUP((uint32_t)vstart);
    for (; p + PGSIZE <= (uint8_t *)vend; p += PGSIZE) {
        kfree(p);
    }
}

uint8_t *kalloc(void) {
    struct page_t *p;

    p = kmem.freelist;
    if (p)
        kmem.freelist = p->next;
    return (uint8_t *)p;
}

static struct kmap {
    void *va;
    uint32_t pa_start;
    uint32_t pa_end;
    int perm;
} kmap[] = {
    { (void *)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
    { (void *)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
    { (void *)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
    { (void *)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
};

pde_t* setupkvm(void) {
    pde_t *pgdir = 0;
    kmap[2].pa_end = (uint32_t)mem_end;
    pgdir = (pde_t *)kalloc();
    if(pgdir) {
        memset(pgdir, 0, PGSIZE);
        if (P2V(mem_end) > (void*)DEVSPACE) {
            panic("phy_mem_end too high");
        }
        for(struct kmap *m = kmap; m < &kmap[NELEM(kmap)]; m++) {
            map_pages(pgdir,
                m->va,
                m->pa_end - m->pa_start,
                (uint32_t)m->pa_start,
                m->perm
            );
        }
        return pgdir;
    } else {
        return 0;
    }
}

int map_pages(pde_t *pgdir, void *va, size_t size, uint32_t pa, int perm) {
    uint8_t *a, *last;
    pte_t *pte;

    a = (uint8_t *)PGROUNDDOWN((uint32_t)va);
    last = (uint8_t *)PGROUNDDOWN(((uint32_t)va) + size - 1);
    while (1) {
        if((pte = walkpgdir(pgdir, a, 1)) == 0)
            return -1;
        if(*pte & PTE_P)
            panic("remap");
        *pte = pa | perm | PTE_P;
        if(a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    
    return 0;
}

pte_t *walkpgdir(pde_t *pgdir, const void *va, int alloc) {
    pde_t *pde;
    pte_t *pgtab;

    pde = &pgdir[PDX(va)];
    if(*pde & PTE_P) {
        pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
    } else {
        if(!alloc || (pgtab = (pte_t*)kalloc()) == 0) {
            return 0;
        }
            
        // Make sure all those PTE_P bits are zero.
        memset(pgtab, 0, PGSIZE);
        // The permissions here are overly generous, but they can
        // be further restricted by the permissions in the page table
        // entries, if necessary.
        *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
    }
  return &pgtab[PTX(va)];
}

// Switch to the kernel page table.
void switchkvm(void) {
    lcr3(V2P(kpgdir));
}
