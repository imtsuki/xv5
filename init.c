#include "defs.h"
#include "include/types.h"
#include "include/stdio.h"
#include "include/string.h"
#include "include/x86.h"
#include "traps.h"
#include "include/mmu.h"

void init(void) {
    copy_bootparams();
    video_init();
    vga_console_init();
    memory_init();
    
    idt_init();
    pic_init();
    kbd_init();
    timer_init();
    
    
    
    int32_t i = 1;
    int32_t *vaddr = &i;
    int32_t *laddr = (int32_t *)((uint32_t)vaddr);
    vga_console_printf("\n\nChecking VA mapping...\n\n");
    vga_console_printf("Value = %d at VA: 0x%08x\n",*vaddr, vaddr);
    vga_console_printf("Value = %d at PA: 0x%08x\n",*laddr, laddr);
    vga_console_printf("\nNow change the value from %d to %d by VA\n\n", *vaddr, 2);
    *vaddr = 2;
    vga_console_printf("After value = %d at VA: 0x%08x\n",*vaddr, vaddr);
    vga_console_printf("After value = %d at PA: 0x%08x\n",*laddr, laddr);
    run_scheduler();
}

void copy_bootparams(void) {

}

pde_t temp_page_dir[];

__attribute__((__aligned__(PGSIZE)))
pde_t temp_page_dir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map Video Buffer
  [0xfd000000>>PDXSHIFT] = (0xfd000000) | PTE_P | PTE_W | PTE_PS,
};