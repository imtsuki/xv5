#include "include/mmu.h"
.text

.global trap_entry
trap_entry:
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal
    movw $(SEG_KDATA<<3), %ax
    movw %ax, %ds
    movw %ax, %es
    pushl %esp
    call trap
    addl $4, %esp

.globl trapret
trapret:
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds
    addl $0x8, %esp  # trapno and errcode
    iret