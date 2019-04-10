#include "include/x86.h"
#include "defs.h"
void panic(char *msg) {
    cli();
    vga_console_printf(msg);
    while (1) {
        asm volatile("hlt");
    }
}