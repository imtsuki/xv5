.globl entry
entry:
	.code16
	mov $0, %ax
	mov %ax, %ss
	mov $0x7c00, %sp
	mov %ax, %ds
	mov %ax, %es
	mov $msg, %si
putloop:
	mov (%si), %al
	add $1, %si
	cmp $0, %al
	je fin
	mov $0x0e, %ah
	mov $15, %bx
	int $0x10
	jmp putloop
fin:
	hlt
	jmp fin
msg:
	.asciz "\n\nhello, world\n"