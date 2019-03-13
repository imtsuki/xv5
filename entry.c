#include "type.h"

#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH 320
#define VRAM 0xa0000

extern uint8_t _binary_hankaku_bin_start;
extern uint8_t _binary_hankaku_bin_end;


void write_pixel(uint32_t x, uint32_t y, uint8_t color) {
    uint8_t *dst = (uint8_t *)(VRAM + SCREEN_WIDTH * y + x);
    *dst = color;
}

void putfont(uint32_t x, uint32_t y, int8_t c, uint8_t *font) {
    for (int i = 0; i < 16; i++) {
        char *p = (char *)(VRAM + (y + i) * SCREEN_WIDTH + x);
        char d = font[i];
        if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
    }
}

void init() {
    uint8_t *hankaku = &_binary_hankaku_bin_start;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < 320; x++) {
            write_pixel(x, y, 8);
        }
    }
    putfont(8, 8, 15, hankaku + 'H' * 16);
    putfont(16, 8, 15, hankaku + 'e' * 16);
    putfont(24, 8, 15, hankaku + 'l' * 16);
    putfont(32, 8, 15, hankaku + 'l' * 16);
    putfont(40, 8, 15, hankaku + 'o' * 16);
}