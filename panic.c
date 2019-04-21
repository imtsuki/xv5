#include "include/x86.h"
#include "defs.h"
void panic(char *msg) {
    cli();
    vga_console_printf("panic: %s", msg);
    while (1) {
        asm volatile("hlt");
    }
}