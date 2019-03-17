#include "include/x86.h"
#include "include/types.h"
#include "defs.h"
#include "traps.h"

// Wait for keyboard controller ready.
void wait_kbc(void) {
    while(1) {
        if((inb(0x64) & 0x02) == 0) {
            break;
        }
    }
}

// Initialize keyboard and mouse.
void kbd_init(void) {
    outb(PIC0_IMR, 0xf9);   // enable keyboard
    outb(PIC1_IMR, 0xef);   // enable mouse
    // initialize keyboard
    wait_kbc();
    outb(0x64, 0x60);
    wait_kbc();
    outb(0x60, 0x47);
    // initialize mouse
    wait_kbc();
    outb(0x64, 0xd4);
    wait_kbc();
    outb(0x60, 0xf4);
}