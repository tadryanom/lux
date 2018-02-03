
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <boot.h>

extern uint8_t bootfont[];

void screen_init(vbe_mode_t *);
void screen_redraw();
void screen_lock();
void screen_unlock();
void screen_use_back_buffer();
void screen_use_front_buffer();
void *screen_offset(uint16_t, uint16_t);

void screen_clear(uint32_t);
void screen_drawch(char, uint16_t, uint16_t, uint32_t, uint32_t);
void screen_drawch_transparent(char, uint16_t, uint16_t, uint32_t);
void screen_drawstr(char *, uint16_t, uint16_t, uint32_t, uint32_t);
void screen_drawstr_transparent(char *, uint16_t, uint16_t, uint32_t);


