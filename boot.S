#include "./mmu.h"
.globl entry
.set PROTECTED_MODE_CS, 0x8
.set PROTECTED_MODE_DS, 0x10
entry:
	.code16
	# Disable interrupts
	cli
	# Initialize registers
	mov		$0, %ax
	mov		%ax, %ss
	mov		%ax, %ds
	mov		%ax, %es
video:
	mov $0x13, %al
	mov $0x00, %ah
	int $0x10
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