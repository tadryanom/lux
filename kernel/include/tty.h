
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <boot.h>

#define TTY_COUNT			4

// Original VGA pallette
#define VGA_BLACK			0x000000
#define VGA_BLUE			0x0000AA
#define VGA_GREEN			0x00AA00
#define VGA_CYAN			0x00AAAA
#define VGA_RED				0xAA0000
#define VGA_MAGENTA			0xAA00AA
#define VGA_BROWN			0xAA5500
#define VGA_GRAY			0xAAAAAA
#define VGA_DARK_GRAY			0x555555
#define VGA_BRIGHT_BLUE			0x5555FF
#define VGA_BRIGHT_GREEN		0x55FF55
#define VGA_BRIGHT_CYAN			0x55FFFF
#define VGA_BRIGHT_RED			0xFF5555
#define VGA_BRIGHT_MAGENTA		0xFF55FF
#define VGA_YELLOW			0xFFFF55
#define VGA_WHITE			0xFFFFFF

typedef struct tty_t
{
	uint32_t bg, fg;
	uint16_t x_pos, y_pos;
	char cursor_visible;
	char lock;
	char *buffer;
} tty_t;

extern uint8_t bootfont[];

tty_t *ttys;

void screen_init(vbe_mode_t *);
void screen_redraw();
void *screen_offset(uint16_t, uint16_t);
void screen_lock();
void screen_unlock();

void screen_clear(uint32_t);
void screen_drawch(char, uint16_t, uint16_t, uint32_t, uint32_t);
void screen_drawch_transparent(char, uint16_t, uint16_t, uint32_t);
void screen_drawstr(char *, uint16_t, uint16_t, uint32_t, uint32_t);
void screen_drawstr_transparent(char *, uint16_t, uint16_t, uint32_t);
void screen_fill_rect(uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

void tty_switch(size_t);
void tty_scroll(size_t);
void tty_redraw(size_t);
void tty_put(char, size_t);
void tty_write(char *, size_t, size_t);
void tty_writestr(char *, size_t);



