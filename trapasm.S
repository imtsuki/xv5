.text

.global trap_entry
trap_entry:
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal
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