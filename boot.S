#include "include/mmu.h"
.globl entry
.set PROTECTED_MODE_CS, 0x8
.set PROTECTED_MODE_DS, 0x10
.set VESA_MODE, 0x11B
entry:
	.code16
	# Disable interrupts
	cli
	cld
	# Initialize registers
	mov $0, %ax
	mov %ax, %ss
	mov %ax, %ds
	mov %ax, %es
	
video:
	mov $0x9000, %di
	mov $0x115, %cx
	mov $0x4f01, %ax
	int $0x10
	mov	$0x4115, %bx
	mov $0x4f02, %ax
	int $0x10
get_memory_layout:
	mov $0x0, %ax
	mov %ax, %es
	movl $0, 0x8000
	xor %bx, %bx
	mov $0x8004, %di
start_get_memory_layout:
	mov $0xe820, %ax
	mov $20, %cx
	movl $0x534D4150, %edx
	int $0x15
	jnc cont_get_memory_layout
	jmp finish_get_memory_layout
cont_get_memory_layout:
	add $20, %di
	incl 0x8000
	cmp $0, %bx
	jnz start_get_memory_layout
finish_get_memory_layout:
	# Enable A20
seta20.1:
	inb		$0x64, %al               # Wait for not busy
	testb	$0x2, %al
	jnz		seta20.1
	movb	$0xd1, %al               # 0xd1 -> port 0x64
	outb	%al, $0x64
seta20.2:
	inb		$0x64, %al               # Wait for not busy
	testb	$0x2, %al
	jnz		seta20.2
	movb	$0xdf, %al               # 0xdf -> port 0x60
	outb	%al, $0x60
	# Switch to protect mode
	lgdt	gdtdesc					# load GDT

	movl	%cr0, %eax
	orl		$0x1, %eax
	movl	%eax, %cr0


	# jump to 32-bit instruction
	# 清空流水线

	ljmp	$PROTECTED_MODE_CS, $protcseg
	.code32
protcseg:
  # Set up the protected-mode data segment registers
	movw	$PROTECTED_MODE_DS, %ax # Our data segment selector
	movw	%ax, %ds                # -> DS: Data Segment
	movw	%ax, %es                # -> ES: Extra Segment
	movw	%ax, %fs                # -> FS
	movw	%ax, %gs                # -> GS
	movw	%ax, %ss                # -> SS: Stack Segment
	call bootmain
spin:
	hlt
	jmp spin
.p2align 2
gdt:
	SEG_NULL
	SEG(STA_X|STA_R, 0x0, 0xffffffff) # code seg
	SEG(STA_W, 0x0, 0xffffffff) # data seg
# GDTR register 48-bit
gdtdesc:
  .word   0x17                            # sizeof(gdt) - 1
  .long   gdt                             # address of gdt