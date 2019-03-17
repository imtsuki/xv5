#ifndef MMU_H
#define MMU_H

#ifdef __ASSEMBLER__

#define SEG_NULL                                                \
    .word 0, 0;                                                 \
    .byte 0, 0, 0, 0
#define SEG(type, base, limit)                                  \
    .word (((limit) >> 12) & 0xffff), ((base) & 0xffff);        \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),             \
        (0xC0 | (((limit) >> 28) & 0xf)), (((base) >> 24) & 0xff)
#else

#endif // __ASSEMBLER__

#define STA_X		0x8	    // Executable segment
#define STA_W		0x2	    // Writeable (non-executable segments)
#define STA_R		0x2	    // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

#ifndef __ASSEMBLER__

struct gatedesc {
	unsigned off_15_0 : 16;   // low 16 bits of offset in segment
	unsigned cs : 16;        // segment selector
	unsigned args : 5;        // # args, 0 for interrupt/trap gates
	unsigned rsv1 : 3;        // reserved(should be zero I guess)
	unsigned type : 4;        // type(STS_{TG,IG32,TG32})
	unsigned s : 1;           // must be 0 (system)
	unsigned dpl : 2;         // descriptor(meaning new) privilege level
	unsigned p : 1;           // Present
	unsigned off_31_16 : 16;  // high bits of offset in segment
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
    //   see section 9.6.1.3 of the i386 reference: "The difference between
    //   an interrupt gate and a trap gate is in the effect on IF (the
    //   interrupt-enable flag). An interrupt that vectors through an
    //   interrupt gate resets IF, thereby preventing other interrupts from
    //   interfering with the current interrupt handler. A subsequent IRET
    //   instruction restores IF to the value in the EFLAGS image on the
    //   stack. An interrupt through a trap gate does not change IF."
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//	  the privilege level required for software to invoke
//	  this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)			\
{								\
	(gate).off_15_0 = (uint32_t) (off) & 0xffff;		\
	(gate).cs = (sel);					\
	(gate).args = 0;					\
	(gate).rsv1 = 0;					\
	(gate).type = (istrap) ? STS_TG32 : STS_IG32;	\
	(gate).s = 0;					\
	(gate).dpl = (d);					\
	(gate).p = 1;					\
	(gate).off_31_16 = (uint32_t) (off) >> 16;		\
}

// Set up a call gate descriptor.
#define SETCALLGATE(gate, sel, off, d)           	        \
{								\
	(gate).off_15_0 = (uint32_t) (off) & 0xffff;		\
	(gate).cs = (sel);					\
	(gate).args = 0;					\
	(gate).rsv1 = 0;					\
	(gate).type = STS_CG32;				\
	(gate).s = 0;					\
	(gate).dpl = (d);					\
	(gate).p = 1;					\
	(gate).off_31_16 = (uint32_t) (off) >> 16;		\
}

#endif // __ASSEMBLER__
#endif // MMU_H