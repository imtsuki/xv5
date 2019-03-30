#include "defs.h"
#include "include/mmu.h"

void memory_init(void) {
    struct e820_table *base = (struct e820_table *)0x8000;
    vga_console_printf("Memory Layout\n\n");
    vga_console_printf("total %d\n\n", base->nr_entries);
    vga_console_printf("address    length     type\n\n");
    for (int i = 0; i < base->nr_entries; i++) {
        vga_console_printf("0x%08x 0x%08x %s\n", base->entries[i].base_addr_low, base->entries[i].length_low, (base->entries[i].type == 1) ? "RAM" : "RESERVED");
    }
    vga_console_printf("\nSize of usable memory above 1MB: %dMB", base->entries[3].length_low / 0x100000);
}