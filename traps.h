#ifndef TRAPS_H
#define TRAPS_H

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define T_IRQ0          32
#define T_SYSCALL       64

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_MOUSE       12
#define IRQ_IDE         14
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31

#ifndef __ASSEMBLER__

#include "include/types.h"

struct trapframe {
  // registers as pushed by pusha
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t oesp;      // useless & ignored
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  // rest of trap frame
  uint16_t gs;
  uint16_t padding1;
  uint16_t fs;
  uint16_t padding2;
  uint16_t es;
  uint16_t padding3;
  uint16_t ds;
  uint16_t padding4;
  uint32_t trapno;

  // below here defined by x86 hardware
  uint32_t err;
  uint32_t eip;
  uint16_t cs;
  uint16_t padding5;
  uint32_t eflags;

  // below here only when crossing rings, such as from user to kernel
  uint32_t esp;
  uint16_t ss;
  uint16_t padding6;
};
#endif

#endif