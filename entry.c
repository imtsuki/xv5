#include "defs.h"
#include "include/types.h"
#include "include/stdio.h"
#include "include/x86.h"
#include "traps.h"


void init(void) {
    video_init();
    idt_init();
    pic_init();
    kbd_init();

    vga_puts(8, 8, 15, "Hello, Sekai");
    for (;;) {
        asm volatile("hlt");
    }
}