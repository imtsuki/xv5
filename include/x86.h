#ifndef X86_H
#define X86_H

#include "types.h"
#include "mmu.h"

static inline uint8_t inb(int port) {
    uint8_t data;
    asm volatile("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void outb(int port, uint8_t data) {
    asm volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static inline void insl(int port, void *addr, int cnt) {
    asm volatile("cld\n\trepne\n\tinsl"
             : "=D" (addr), "=c" (cnt)
             : "d" (port), "0" (addr), "1" (cnt)
             : "memory", "cc");
}

static inline void sti(void) {
    asm volatile("sti");
}
static inline void cli(void) {
    asm volatile("cli");
}

static uint64_t
make_idtr_operand (uint16_t limit, uint32_t base) {
  return limit | ((uint64_t) (uint32_t) base << 16);
}

static inline void
lidt(struct gatedesc *p, int size)
{
  volatile uint16_t pd[3];

  pd[0] = size-1;
  pd[1] = (uint32_t)p;
  pd[2] = (uint32_t)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

#endif // X86_H