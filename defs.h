#ifndef DEFS_H
#define DEFS_H

#include "include/types.h"

// video.c
void video_init(void);
void write_pixel(uint32_t x, uint32_t y, uint8_t b, uint8_t g, uint8_t r);
void vga_putchar(uint32_t x, uint32_t y, uint32_t c, uint8_t *font);
void vga_puts(int x, int y, char c, char *s);

// trap.c
void idt_init(void);
void pic_init(void);

// kbd.c
void kbd_init(void);
void wait_kbc(void);

#endif