#include "defs.h"
#include "include/mmu.h"
#include "include/types.h"
#include "include/string.h"
#include "include/x86.h"
#include "cpu.h"
#include "proc.h"

pde_t *kpgdir;

uint8_t *mem_start, *mem_end;
size_t mem_length;
extern uint8_t data[], end[];

extern struct cpu_t cpu;

struct page_t {
    struct page_t *next;
};

struct {
  struct page_t *freelist;
} kmem;

void memory_init(void) {
    cpu.proc = 0;
    seg_init();
    vga_console_printf("Initializing memory...\n");
    get_memory_layout();
    free_range(end, P2V(4 * 1024 * 1024));
    kpgdir = setupkvm();
    switchkvm();
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
    cpu.gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
    cpu.gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
    cpu.gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
    cpu.gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
    lgdt(cpu.gdt, sizeof(cpu.gdt));
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

void
inituvm(pde_t *pgdir, char *init, size_t sz) {
  uint8_t *mem;

  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  map_pages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}

void switchuvm(struct proc_t *p) {
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");
  cli();
  memset(&(cpu.tss), 0, sizeof(cpu.tss));
  cpu.gdt[SEG_TSS] = SEG16(STS_T32A, &(cpu.tss),
                                sizeof(cpu.tss)-1, 0);
  cpu.gdt[SEG_TSS].s = 0;
  cpu.tss.ts_ss0 = SEG_KDATA << 3;
  cpu.tss.ts_esp0 = (uint32_t)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  cpu.tss.ts_iomb = (uint16_t) 0xFFFF;
  ltr(SEG_TSS << 3);
  lcr3(V2P(p->pgdir));  // switch to process's address space
  sti();
}

pde_t* copyuvm(pde_t *pgdir, uint32_t sz) {
  pde_t *d;
  pte_t *pte;
  uint32_t pa, i, flags;
  uint8_t *mem;

  if((d = setupkvm()) == 0)
    return 0;
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char*)P2V(pa), PGSIZE);
    if(map_pages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
      kfree(mem);
      goto bad;
    }
  }
  return d;

bad:
  //freevm(d);
  return 0;
}