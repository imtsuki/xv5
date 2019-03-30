#include "include/x86.h"
#include "include/types.h"
#include "defs.h"
#include "traps.h"
#include "kbd.h"

// Wait for keyboard controller ready.
void wait_kbc(void) {
    while(1) {
        if((inb(0x64) & 0x02) == 0) {
            break;
        }
    }
}

// Initialize keyboard and mouse.
void kbd_init(void) {
    outb(PIC0_IMR, 0xf9);   // enable keyboard
    outb(PIC1_IMR, 0xef);   // enable mouse
    // initialize keyboard
    wait_kbc();
    outb(0x64, 0x60);
    wait_kbc();
    outb(0x60, 0x47);
    // initialize mouse
    wait_kbc();
    outb(0x64, 0xd4);
    wait_kbc();
    outb(0x60, 0xf4);
}

// Get a char from keyboard.
int kbdgetc(void) {

    int c;
	uint8_t stat, data;
	static uint32_t shift = 0;

	stat = inb(KBSTATP);
	if ((stat & KBS_DIB) == 0)
		return -1;
	// Ignore data from mouse.
	if (stat & KBS_TERR)
		return -1;

	data = inb(KBDATAP);

	if (data == 0xE0) {
		// E0 escape character
		shift |= E0ESC;
		return 0;
	} else if (data & 0x80) {
		// Key released
		data = (shift & E0ESC ? data : data & 0x7F);
		shift &= ~(shiftcode[data] | E0ESC);
		return 0;
	} else if (shift & E0ESC) {
		// Last character was an E0 escape; or with 0x80
		data |= 0x80;
		shift &= ~E0ESC;
	}
	c = charcode[0][data];

	return c;
}

void kbdintr() {
    char c = kbdgetc();
    if (c != 0) {
        vga_console_putc(c);
    }
}