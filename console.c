#define CONSOLE_WIDTH 98
#define CONSOLE_HEIGHT 36
#include "defs.h"
#include "include/stdio.h"
char buffer[CONSOLE_HEIGHT][CONSOLE_WIDTH];

// cursor
int x = 0;
int y = 0;

void vga_console_draw_ascii(int row, int column, char c) {
    buffer[row][column] = c;
    vga_boxfill(8 + column * 8, 8 + row * 16, 8, 16, rgb(0, 0, 0));
    vga_draw_ascii(8 + column * 8, 8 + row * 16, 255, c);
}

void vga_console_init() {
    x = 0;
    y = 0;
    for (int i = 0; i < CONSOLE_HEIGHT; i++) {
        for (int j = 0; j < CONSOLE_WIDTH; j++) {
            vga_console_draw_ascii(i, j, ' ');
        }
    }
}

void vga_console_putc(char c) {
    if (c == '\n') {
        y += 1;
        x = 0;
        return;
    } else if (c == '\b' && x != 0) {
        x--;
        vga_console_draw_ascii(y, x, ' ');
        return;
    }
    vga_console_draw_ascii(y, x, c);
    x++;
    if (x >= CONSOLE_WIDTH) {
        x = 0;
        y += 1;
    }
    if (y >= CONSOLE_HEIGHT) {
        y = 0;
    }
}

int vga_console_printf(char *fmt, ...) {
    static char buf[1024];
    static int n = 1024;
    va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = vsnprintf(buf, n, fmt, ap);
	va_end(ap);
    for (int i = 0; buf[i] != '\0'; i++) {
        vga_console_putc(buf[i]);
    }
	return rc;
}