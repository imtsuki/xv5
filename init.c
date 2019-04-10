#include "defs.h"
#include "include/types.h"
#include "include/stdio.h"
#include "include/string.h"
#include "include/x86.h"
#include "traps.h"
#include "include/mmu.h"

void init(void) {
    video_init();
    vga_console_init();
    memory_init();
    idt_init();
    pic_init();
    kbd_init();
    timer_init();
    
    
    vga_console_printf("\nWelcome to BuggySys.\n\n>");
    run_scheduler();
}

pde_t temp_page_dir[];

__attribute__((__aligned__(PGSIZE)))
pde_t temp_page_dir[NPDENTRIES] = {
    // Map VA's [0, 4MB) to PA's [0, 4MB)
    [0] = (0) | PTE_P | PTE_W | PTE_PS,
    // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
    [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
    // Map Video Buffer
    [DEVSPACE>>PDXSHIFT] = (DEVSPACE) | PTE_P | PTE_W | PTE_PS,
};