.global entry
entry:
    mov $0x400000, %esp
    mov $init, %eax
    jmp *%eax