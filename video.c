#include "include/types.h"
#include "include/vesa.h"

uint32_t SCREEN_HEIGHT = 1024;
uint32_t SCREEN_WIDTH = 1280;
uint8_t *VRAM = (uint8_t *)0xa0000;

extern uint8_t _binary_hankaku_bin_start;
extern uint8_t _binary_hankaku_bin_end;


void write_pixel(uint32_t x, uint32_t y, uint8_t b, uint8_t g, uint8_t r) {
    uint8_t *dst = VRAM + 3 *(SCREEN_WIDTH * y + x);
    *dst = b;
    *(dst + 1) = g;
    *(dst + 2) = r;
}

void vga_putchar(uint32_t x, uint32_t y, uint32_t c, uint8_t *font) {
    for (int i = 0; i < 16; i++) {
        uint8_t *p = VRAM + 3 * ((y + i) * SCREEN_WIDTH + x);
        char d = font[i];
        if ((d & 0x80) != 0) { p[0] = p[1] = p[2] = c; }
		if ((d & 0x40) != 0) { p[3] = p[4] = p[5] = c; }
		if ((d & 0x20) != 0) { p[6] = p[7] = p[8] = c; }
		if ((d & 0x10) != 0) { p[9] = p[10] = p[11] = c; }
		if ((d & 0x08) != 0) { p[12] = p[13] = p[14] = c; }
		if ((d & 0x04) != 0) { p[15] = p[16] = p[17] = c; }
		if ((d & 0x02) != 0) { p[18]  = p[19] = p[20]= c; }
		if ((d & 0x01) != 0) { p[21]  = p[22] = p[23]= c; }
    }
}

void vga_puts(int x, int y, char c, char *s) {
    uint8_t *hankaku = &_binary_hankaku_bin_start;
    for (; *s != '\0'; s++) {
        vga_putchar(x, y, c, hankaku + *s * 16);
        x += 8;
    }
}

void video_init(void) {
    SCREEN_HEIGHT = modeInfo->wYResolution;
    SCREEN_WIDTH = modeInfo->wXResolution;
    VRAM = (uint8_t *)(modeInfo->dwPhysicalBasePointer);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            write_pixel(x, y, 0xFF, 0xFF, 0xFF);
        }
    }
}