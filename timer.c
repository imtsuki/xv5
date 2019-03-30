#include "defs.h"
#include "include/x86.h"
#include "include/types.h"
#include "traps.h"

void timer_init(void) {
    outb(0x43, 0x34);
    outb(0x40, 0x9c);
    outb(0x40, 0x2e);
    outb(PIC0_IMR, 0xf8);
}

void timerintr(void) {

}