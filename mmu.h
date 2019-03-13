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

#endif // MMU_H