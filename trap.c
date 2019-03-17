#include "include/mmu.h"
#include "include/x86.h"
#include "include/stdio.h"
#include "defs.h"
#include "traps.h"

struct gatedesc idt[256];
extern uint32_t vectors[];
void trap_entry(void);

void pic_init(void) {
    outb(PIC0_IMR,  0xff  );
    outb(PIC1_IMR,  0xff  );

    outb(PIC0_ICW1, 0x11  );
    outb(PIC0_ICW2, 0x20  );
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, 0x01  );

    outb(PIC1_ICW1, 0x11  );
    outb(PIC1_ICW2, 0x28  );
    outb(PIC1_ICW3, 2     );
    outb(PIC1_ICW4, 0x01  );

    outb(PIC0_IMR,  0xfb  );
    outb(PIC1_IMR,  0xff  );
    sti();
}

void idt_init(void) {
    for (int i = 0; i <= 256; i++) {
        SETGATE(idt[i], 0, 1 << 3, vectors[i], 0);
    }
    lidt(idt, sizeof(idt));
}

void trap(struct trapframe *tf) {
    static int y = 16;
    static int x = 8;
    extern uint32_t SCREEN_HEIGHT;
    unsigned char data;
    char s[4];
    
    switch (tf->trapno) {
        case T_IRQ0 + IRQ_KBD:
            outb(PIC0_OCW2, 0x61);
            data = inb(0x60);
            snprintf(s, 4, "%02x", data);
            vga_puts(x, y, 10, s);
            y += 20;
            if (y > SCREEN_HEIGHT) {
                y = 16;
                x += 40;
            }
        break;
        case T_IRQ0 + IRQ_MOUSE:
            outb(PIC1_OCW2, 0x64);
            outb(PIC0_OCW2, 0x62);
            data = inb(0x60);
            snprintf(s, 4, "%02x", data);
            vga_puts(x, y, 10, s);
            y += 20;
            if (y > SCREEN_HEIGHT) {
                y = 16;
                x += 40;
            }
        break;
    }
    
}