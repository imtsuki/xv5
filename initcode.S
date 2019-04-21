#include "syscall.h"
#include "traps.h"


# exec(init, argv)
.globl start
start:
  mov $SYS_fork, %eax
  int $T_SYSCALL
  int $T_SYSCALL
spin:
  jmp spin