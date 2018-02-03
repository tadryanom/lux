
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <boot.h>
#include <mm.h>
#include <kprintf.h>
#include <screen.h>
#include <string.h>

#define BACK_BUFFER			0
#define FRONT_BUFFER			1

uint16_t width, height, pitch;
uint16_t width_chars, height_chars, x_pos, y_pos;
uint32_t framebuffer;
size_t screen_size, screen_size_dwords, screen_size_sse2;
size_t back_buffer;
char buffer_in_use, lock_flag;

// screen_init(): Initializes the screen
// Param:	vbe_mode_t *vbe_mode - VESA mode information
// Return:	Nothing

void screen_init(vbe_mode_t *vbe_info)
{
	kprintf("screen: using VESA framebuffer for output, %dx%dx%dbpp\n", vbe_info->width, vbe_info->height, vbe_info->bpp);
	kprintf("screen: framebuffer is at 0x%xd\n", vbe_info->framebuffer);

	width = vbe_info->width;
	height = vbe_info->height;
	pitch = vbe_info->pitch;
	framebuffer = vbe_info->framebuffer;

	width_chars = (width / 8) - 2;
	height_chars = (height / 16) - 2;
	x_pos = 0;
	y_pos = 0;

	screen_size = height*pitch;
	screen_size_dwords = screen_size / 4;

	// for SSE2 copying, because we have 8 SSE registers ...
	// ... and each register is 16 bytes, so 8*16 = 128
	screen_size_sse2 = (screen_size+127) / 128;

	// map the framebuffer
	vmm_map(HW_FRAMEBUFFER, framebuffer, (screen_size / PAGE_SIZE) + 1, PAGE_PRESENT | PAGE_RW);

	// allocate a back buffer
	back_buffer = pmm_alloc((screen_size / PAGE_SIZE) + 1);
	vmm_map(SW_FRAMEBUFFER, back_buffer, (screen_size / PAGE_SIZE) + 1, PAGE_PRESENT | PAGE_RW);

	buffer_in_use = BACK_BUFFER;
	lock_flag = 0;

	screen_clear(0x000000);
}

// screen_redraw(): Redraws the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_redraw()
{
	if(buffer_in_use == BACK_BUFFER)
	{
		if(lock_flag == 0)
			sse2_copy((void*)HW_FRAMEBUFFER, (void*)SW_FRAMEBUFFER, screen_size_sse2);
	}
}

// screen_lock(): Locks the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_lock()
{
	lock_flag = 1;
}

// screen_unlock(): Unlocks the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_unlock()
{
	lock_flag = 0;
}

// screen_use_back_buffer(): Switches to the back buffer
// Param:	Nothing
// Return:	Nothing

inline void screen_use_back_buffer()
{
	buffer_in_use = BACK_BUFFER;
}

// screen_use_front_buffer(): Switches to the front buffer
// Param:	Nothing
// Return:	Nothing

inline void screen_use_front_buffer()
{
	buffer_in_use = FRONT_BUFFER;
}

// screen_offset(): Returns offset of a pixel
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	void * - pointer in current framebuffer

void *screen_offset(uint16_t x, uint16_t y)
{
	void *ptr = (void*)(y * pitch) + (x << 2);	// x * 4

	if(buffer_in_use == FRONT_BUFFER)
		return ptr + HW_FRAMEBUFFER;
	else if(buffer_in_use == BACK_BUFFER)
		return ptr + SW_FRAMEBUFFER;

	else
	{
		// we should never be here!
		kprintf("Framebuffer being used is undefined.\n");
		kprintf("Assuming hardware framebuffer...\n");
		buffer_in_use = FRONT_BUFFER;
		return ptr + HW_FRAMEBUFFER;
	}
}

// screen_clear(): Clears the screen to a solid color
// Param:	uint32_t color - color value
// Return:	Nothing

void screen_clear(uint32_t color)
{
	uint32_t *screen = screen_offset(0, 0);

	size_t i = 0;
	while(i < screen_size_dwords)
	{
		screen[i] = color;
		i++;
	}

	screen_redraw();
}

// screen_drawch(): Draws a character
// Param:	char character - character
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	uint32_t fg - foreground
// Param:	uint32_t bg - background
// Return:	Nothing

void screen_drawch(char character, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg)
{
	void *ptr = screen_offset(x, y);
	uint32_t *pixels = (uint32_t*)ptr;

	uint8_t *font = bootfont + ((int)character << 4);
	uint8_t fontbyte = font[0];

	int i = 0, j = 0;

	while(j < 16)
	{
		while(i < 8)
		{
			if(fontbyte & 0x80)
				pixels[i] = fg;
			else
				pixels[i] = bg;

			fontbyte <<= 1;
			i++;
		}

		j++;
		fontbyte = font[j];

		ptr += pitch;
		pixels = (uint32_t*)ptr;
		i = 0;
	}
}

// screen_drawch_transparent(): Draws a character with transparent background
// Param:	char character - character
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	uint32_t fg - foreground
// Return:	Nothing

void screen_drawch_transparent(char character, uint16_t x, uint16_t y, uint32_t fg)
{
	void *ptr = screen_offset(x, y);
	uint32_t *pixels = (uint32_t*)ptr;

	uint8_t *font = bootfont + ((int)character << 4);
	uint8_t fontbyte = font[0];

	int i = 0, j = 0;

	while(j < 16)
	{
		while(i < 8)
		{
			if(fontbyte & 0x80)
				pixels[i] = fg;

			fontbyte <<= 1;
			i++;
		}

		j++;
		fontbyte = font[j];

		ptr += pitch;
		pixels = (uint32_t*)ptr;
		i = 0;
	}
}

// screen_drawstr(): Draws a string
// Param:	char *string - string to render
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	uint32_t fg - foreground
// Param:	uint32_t bg - background
// Return:	Nothing

void screen_drawstr(char *string, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg)
{
	uint16_t cx = x, cy = y;

	while(string[0] != 0)
	{
		if(string[0] == 13)		// carriage
		{
			cx = x;
			string++;
			continue;
		}

		if(string[0] == 10)		// newline
		{
			cx = x;
			cy += 16;
			string++;
			continue;
		}

		else
		{
			screen_drawch(string[0], cx, cy, fg, bg);
			cx += 8;
			string++;
			continue;
		}
	}

	screen_redraw();
}

// screen_drawstr_transparent(): Draws a string with transparent background
// Param:	char *string - string to render
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	uint32_t fg - foreground
// Return:	Nothing

void screen_drawstr_transparent(char *string, uint16_t x, uint16_t y, uint32_t fg)
{
	uint16_t cx = x, cy = y;

	while(string[0] != 0)
	{
		if(string[0] == 13)		// carriage
		{
			cx = x;
			string++;
			continue;
		}

		if(string[0] == 10)		// newline
		{
			cx = x;
			cy += 16;
			string++;
			continue;
		}

		else
		{
			screen_drawch_transparent(string[0], cx, cy, fg);
			cx += 8;
			string++;
			continue;
		}
	}

	screen_redraw();
}




