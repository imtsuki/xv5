#include "include/mmu.h"
.globl _start
_start = V2P(entry) 

.global entry
entry:
# jmp fuck
    movl    %cr4, %eax
    orl     $(CR4_PSE), %eax
    movl    %eax, %cr4
    # Set page directory
    movl    $(V2P(temp_page_dir)), %eax
    movl    %eax, %cr3
    # Turn on paging.
    movl    %cr0, %eax
    orl     $(CR0_PG|CR0_WP), %eax
    movl    %eax, %cr0

  # Set up the stack pointer.
  movl $(stack + KSTACKSIZE), %esp
fuck:
    # mov $0x400000, %esp
    mov $init, %eax
    jmp *%eax

.comm stack, KSTACKSIZE