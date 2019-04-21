#ifndef MMU_H
#define MMU_H

#ifndef __ASSEMBLER__
#include "types.h"
#endif

// Eflags register
#define FL_IF           0x00000200      // Interrupt Enable

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_WP          0x00010000      // Write Protect
#define CR0_PG          0x80000000      // Paging
#define CR4_PSE         0x00000010      // Page size extension

/*********************** Memory Layout *************************/

#include "../config.h"

#ifdef __ASSEMBLER__

#define V2P(x) ((x) - KERNBASE)
#define P2V(x) ((x) + KERNBASE)

#else

#define V2P(a) (((uint32_t) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define E820_MAX 128
struct e820_table {
    uint32_t nr_entries;
    struct {
        uint32_t base_addr_low;
        uint32_t base_addr_high;
        uint32_t length_low;
        uint32_t length_high;
        uint32_t type;
    } __attribute__((packed))
    entries[E820_MAX];
};

#endif

/************************** Segment ****************************/

#ifdef __ASSEMBLER__

#define SEG_NULL                                                \
    .word 0, 0;                                                 \
    .byte 0, 0, 0, 0
#define SEG(type, base, limit)                                  \
    .word (((limit) >> 12) & 0xffff), ((base) & 0xffff);        \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),             \
        (0xC0 | (((limit) >> 28) & 0xf)), (((base) >> 24) & 0xff)
#else

struct segdesc {
  unsigned lim_15_0 : 16;  // Low bits of segment limit
  unsigned base_15_0 : 16; // Low bits of segment base address
  unsigned base_23_16 : 8; // Middle bits of segment base address
  unsigned type : 4;       // Segment type (see STS_ constants)
  unsigned s : 1;          // 0 = system, 1 = application
  unsigned dpl : 2;        // Descriptor Privilege Level
  unsigned p : 1;          // Present
  unsigned lim_19_16 : 4;  // High bits of segment limit
  unsigned avl : 1;        // Unused (available for software use)
  unsigned rsv1 : 1;       // Reserved
  unsigned db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  unsigned g : 1;          // Granularity: limit scaled by 4K when set
  unsigned base_31_24 : 8; // High bits of segment base address
};

// Normal segment
#define SEG(type, base, lim, dpl) (struct segdesc)    \
{ ((lim) >> 12) & 0xffff, (uint32_t)(base) & 0xffff,      \
  ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint32_t)(lim) >> 28, 0, 0, 1, 1, (uint32_t)(base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (uint32_t)(base) & 0xffff,              \
  ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint32_t)(lim) >> 16, 0, 0, 1, 0, (uint32_t)(base) >> 24 }

#endif // __ASSEMBLER__

#define STA_X		0x8	    // Executable segment
#define STA_W		0x2	    // Writeable (non-executable segments)
#define STA_R		0x2	    // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // this process's task state

#define NSEGS     6

#define DPL_USER    0x3     // User DPL

/************************** Page ****************************/

// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

// page directory index
#define PDX(va)         (((uint32_t)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((uint32_t)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint32_t)(pte) &  0xFFF)

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