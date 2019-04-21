#include "include/mmu.h"
#include "include/x86.h"
#include "include/stdio.h"
#include "defs.h"
#include "traps.h"
#include "cpu.h"

extern struct cpu_t cpu;
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

// Init all interrupt gates and perform load idt instruction.
void idt_init(void) {
    for (int i = 0; i <= 256; i++) {
        SETGATE(idt[i], 0, 1 << 3, vectors[i], 0);
    }
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER)
    lidt(idt, sizeof(idt));
}

void trap(struct trapframe *tf) {
    unsigned char data;
    if (tf->trapno == T_SYSCALL) {
        cpu.proc->tf = tf;
        syscall();
        return;
    }
    switch (tf->trapno) {
        case T_IRQ0 + IRQ_TIMER:
            outb(PIC0_OCW2, 0x60);
            timerintr();
        break;
        case T_IRQ0 + IRQ_KBD:
            outb(PIC0_OCW2, 0x61);
            kbdintr();
        break;
        case T_IRQ0 + IRQ_MOUSE:
            outb(PIC1_OCW2, 0x64);
            outb(PIC0_OCW2, 0x62);
            data = inb(0x60);
        break;
        default:
            if (cpu.proc == 0 || (tf->cs&3) == 0) {
                vga_console_printf("Unexpected trap 0x%x %x %x\n", tf->trapno, tf->eip, tf->esp);
                panic("trap");
            }
            vga_console_printf("\nUnexpected user trap 0x%02x: General Protection Fault\n", tf->trapno);
            panic("user trap");
        break;
    }
    if(cpu.proc && cpu.proc->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
        yield();
    
}