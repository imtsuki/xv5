#include "defs.h"
#include "include/types.h"
#include "include/stdio.h"
#include "include/string.h"
#include "include/x86.h"
#include "traps.h"
#include "include/vesa.h"

void init(void) {
    copy_bootparams();
    bss_init();
    video_init();
    idt_init();
    pic_init();
    kbd_init();
    timer_init();
    vga_console_init();
    memory_init();
    run_scheduler();
}

void bss_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
}

void copy_bootparams(void) {

}