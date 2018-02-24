
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <boot.h>

#define TTY_COUNT			6

#define rgb(r,g,b)			((r << 16) + (g << 8) + b)

// Color Pallette
#define BLACK				rgb(0,0,0)
#define RED				rgb(222,56,43)
#define GREEN				rgb(57,181,74)
#define YELLOW				rgb(255,199,6)
#define BLUE				rgb(0,111,184)
#define MAGENTA				rgb(118,38,113)
#define CYAN				rgb(44,181,233)
#define GRAY				rgb(170,170,170)
#define DARK_GRAY			rgb(128,128,128)
#define BRIGHT_RED			rgb(255,0,0)
#define BRIGHT_GREEN			rgb(0,255,0)
#define BRIGHT_YELLOW			rgb(255,255,0)
#define BRIGHT_BLUE			rgb(0,0,255)
#define BRIGHT_MAGENTA			rgb(255,0,255)
#define BRIGHT_CYAN			rgb(0,255,255)
#define WHITE				rgb(255,255,255)

typedef struct tty_t
{
	uint16_t x_pos, y_pos;
	uint8_t attribute;
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

void tty_lock(size_t);
void tty_unlock(size_t);
void tty_switch(size_t);
void tty_scroll(size_t);
void tty_redraw(size_t);
void tty_put(char, size_t);
void tty_write(char *, size_t, size_t);
void tty_writestr(char *, size_t);



