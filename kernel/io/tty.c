
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <boot.h>
#include <mm.h>
#include <kprintf.h>
#include <tty.h>
#include <string.h>
#include <lock.h>
#include <devmgr.h>

uint16_t width, height, pitch;
uint16_t width_chars, height_chars;
size_t framebuffer;
size_t screen_size, screen_size_dwords, screen_size_sse2;
size_t back_buffer;
tty_t *ttys;
size_t current_tty;
char lock_flag;
size_t tty_size;

lock_t tty_mutex = 0;

uint32_t colors[] = {
	BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, GRAY,
	DARK_GRAY, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
	BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, WHITE,
};

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

	framebuffer = (size_t)(vbe_info->framebuffer & 0xFFFFFFFF) & 0xFFFFFFFF;

	width_chars = (width / 8) - 1;
	height_chars = (height / 16) - 1;
	tty_size = width_chars * height_chars * 2;

	screen_size = (size_t)((height*pitch) & 0xFFFFFFFF) & 0xFFFFFFFF;
	screen_size_dwords = screen_size / 4;

	// for SSE2 copying, because we have 8 SSE registers ...
	// ... and each register is 16 bytes, so 8*16 = 128
	screen_size_sse2 = (screen_size+127) / 128;

	// map the framebuffer
	vmm_map(HW_FRAMEBUFFER, framebuffer, (screen_size / PAGE_SIZE) + 1, PAGE_PRESENT | PAGE_RW);

	// allocate a back buffer
	back_buffer = pmm_alloc((screen_size / PAGE_SIZE) + 1);
	vmm_map(SW_FRAMEBUFFER, back_buffer, (screen_size / PAGE_SIZE) + 1, PAGE_PRESENT | PAGE_RW);

	ttys = kcalloc(TTY_COUNT, sizeof(tty_t));

	size_t i = 0, j = 0;
	size_t size = tty_size / 2;
	while(i < TTY_COUNT)
	{
		ttys[i].buffer = kmalloc(width_chars * height_chars * 2);
		j = 0;

		while(j < size)
		{
			ttys[i].buffer[j << 1] = 0;
			ttys[i].buffer[(j << 1) + 1] = 0x07;

			j++;
		}

		ttys[i].attribute = 0x07;	// gray on black, default
		ttys[i].cursor_visible = 1;
		ttys[i].lock = 0;

		i++;
	}

	debug_mode = 1;
	screen_unlock();
	tty_switch(0);

	device_t *device = kmalloc(sizeof(device_t));
	device->category = DEVMGR_CATEGORY_GRAPHICS;
	device->mmio[0].base = (uint64_t)framebuffer;
	device->mmio[0].size = (uint64_t)screen_size;
	devmgr_register(device, "VESA framebuffer");

	kfree(device);
}

// screen_redraw(): Redraws the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_redraw()
{
	if(lock_flag != 0)
		return;

	sse2_copy((void*)(HW_FRAMEBUFFER), (void*)(SW_FRAMEBUFFER), screen_size_sse2);
}

// screen_offset(): Returns offset of a pixel
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	void * - pointer in current framebuffer

void *screen_offset(uint16_t x, uint16_t y)
{
	size_t return_ptr = ((size_t)(y * pitch) + (x << 2));
	void *ptr = (void*)(return_ptr & 0xFFFFFFFF);
	return ptr + SW_FRAMEBUFFER;
}

// screen_lock(): Locks the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_lock()
{
	lock_flag = 1;
}

// screen_unlock(): Unocks the screen
// Param:	Nothing
// Return:	Nothing

inline void screen_unlock()
{
	lock_flag = 0;
}

// screen_clear(): Clears the screen to a solid color
// Param:	uint32_t color - color value
// Return:	Nothing

void screen_clear(uint32_t color)
{
	uint32_t *screen = (uint32_t*)(SW_FRAMEBUFFER);

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
	uint32_t *pixels = (uint32_t*)(ptr);

	uint8_t *font = bootfont + ((int)(character & 0xFF) << 4);
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
		pixels = (uint32_t*)(ptr);
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
	uint32_t *pixels = (uint32_t*)(ptr);

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
		pixels = (uint32_t*)(ptr);
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

// screen_fill_rect(): Draws a rectangle
// Param:	uint16_t x - X coordinate
// Param:	uint16_t y - Y coordinate
// Param:	uint16_t width - width
// Param:	uint16_t height - height
// Param:	uint32_t color - color to fill

void screen_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
	if(!width || !height)
		return;

	void *ptr = screen_offset(x, y);
	uint32_t *pixels = (uint32_t*)(ptr);

	uint16_t i = 0, j = 0;

	while(j < height)
	{
		while(i < width)
		{
			pixels[i] = color;
			i++;
		}

		ptr += pitch;
		pixels = (uint32_t*)(ptr);

		i = 0;
		j++;
	}

	screen_redraw();
}

/* *********************************** *
 *                                     *
 * END OF PIXEL-BASED SCREEN FUNCTIONS *
 * START OF ACTUAL TTY CODE            *
 *                                     *
 * *********************************** */

// tty_lock(): Locks a tty
// Param:	size_t tty - TTY index
// Return:	Nothing

inline void tty_lock(size_t tty)
{
	ttys[tty].lock = 1;
}

// tty_unlock(): Unlocks a tty
// Param:	size_t tty - TTY index
// Return:	Nothing

inline void tty_unlock(size_t tty)
{
	ttys[tty].lock = 0;
	if(current_tty == tty)
		tty_redraw(tty);
}

// tty_switch(): Sets the current tty
// Param:	size_t tty - TTY index
// Return:	Nothing

void tty_switch(size_t tty)
{
	if(tty >= TTY_COUNT)
	{
		kprintf("tty: attempted to switch to non-existant tty %d, ignoring...\n", tty);
		return;
	}
	else
	{
		kprintf("tty: switched to tty %d\n", tty);
		current_tty = tty;
		ttys[tty].lock = 0;
		tty_redraw(tty);
	}
}

// tty_scroll(): Scrolls a terminal
// Param:	size_t tty - terminal to scroll
// Return:	Nothing

void tty_scroll(size_t tty)
{
	char *buffer = ttys[tty].buffer;
	memcpy(buffer, buffer + (width_chars * 2), (width_chars - 1) * height_chars * 2);
	//memset(buffer + ((height_chars-1) * (width_chars * 2)), 0, width_chars * 2);

	uint16_t *last_line = (uint16_t*)(buffer + ((height_chars - 1) * (width_chars * 2)));
	uint16_t i;

	for(i = 0; i < width_chars; i++)
		last_line[i] = (uint16_t)ttys[tty].attribute << 8;

	ttys[tty].x_pos = 0;
	ttys[tty].y_pos = height_chars - 1;

	if(current_tty == tty)
		tty_redraw(tty);
}

// tty_redraw(): Redraws a terminal
// Param:	size_t tty - terminal to redraw
// Return:	Nothing

void tty_redraw(size_t tty)
{
	if(ttys[tty].lock != 0)
		return;

	uint16_t x = 0, y = 0;

	screen_lock();
	screen_clear(colors[(ttys[tty].attribute >> 4) & 0x0F]);

	// now draw character by character
	size_t i = 0;
	//size_t tty_size = width_chars * height_chars * 2;
	while(i < tty_size)
	{
		/*if(ttys[tty].buffer[i] == 0)
		{
			i++;
			continue;
		}*/

		screen_drawch(ttys[tty].buffer[i], x * 8, y * 16, colors[ttys[tty].buffer[i+1] & 0x0F], colors[(ttys[tty].buffer[i+1] >> 4) & 0x0F]);
		x++;
		if(x >= width_chars)
		{
			x = 0;
			y++;
		}

		i += 2;
		continue;
	}

	// draw the cursor if we have to
	if(ttys[tty].cursor_visible != 0)
	{
		i = (ttys[tty].y_pos * width_chars << 1) + (ttys[tty].x_pos << 1);
		screen_fill_rect(ttys[tty].x_pos << 3, (ttys[tty].y_pos << 4) + 14, 8, 2, colors[ttys[tty].buffer[i+1] & 0x0F]);
	}

	screen_unlock();
	screen_redraw();
}

// tty_put(): Puts a character on a terminal
// Param:	char character - character to write
// Param:	size_t tty - terminal to write to
// Return:	Nothing

void tty_put(char character, size_t tty)
{
	if(tty >= TTY_COUNT)
	{
		kprintf("tty: attempt to write to non-existant terminal id %d, ignoring...\n", tty);
		return;
	}

	if(!character)
		return;

	acquire_lock(&tty_mutex);

	size_t buffer_offset = (size_t)((ttys[tty].y_pos * width_chars * 2) + ttys[tty].x_pos * 2) & 0xFFFFFFFF;
	char *buffer = (char*)(ttys[tty].buffer + buffer_offset);

	//buffer[0] = character;

	if(character == 13)
	{
		ttys[tty].x_pos = 0;
	}
	else if(character == 10)
	{
		ttys[tty].x_pos = 0;
		ttys[tty].y_pos++;

		if(ttys[tty].y_pos >= height_chars)
			tty_scroll(tty);

		release_lock(&tty_mutex);
		return;
	}
	else
	{
		buffer[0] = character;
		buffer[1] = ttys[tty].attribute;

		ttys[tty].x_pos++;

		if(ttys[tty].x_pos >= width_chars)
		{
			ttys[tty].x_pos = 0;
			ttys[tty].y_pos++;

			if(ttys[tty].y_pos >= height_chars)
			{
				tty_scroll(tty);
				release_lock(&tty_mutex);
				return;
			}
		}
	}

	if(current_tty == tty)
		tty_redraw(tty);

	release_lock(&tty_mutex);
}

// tty_write(): Writes to a terminal
// Param:	char *string - data to write
// Param:	size_t size - size of data to write
// Param:	size_t tty - terminal to write to
// Return:	Nothing

void tty_write(char *string, size_t size, size_t tty)
{
	if(tty >= TTY_COUNT)
	{
		kprintf("tty: attempt to write to non-existant terminal id %d, ignoring...\n", tty);
		return;
	}

	if(current_tty == tty)
		ttys[tty].lock = 1;		// for performance

	size_t i = 0;
	while(i < size)
	{
		tty_put(string[i], tty);
		i++;
	}

	if(current_tty == tty)
	{
		ttys[tty].lock = 0;
		tty_redraw(tty);
	}
}

// tty_writestr(): Writes an ASCIIZ string to a terminal
// Param:	char *string - string
// Param:	size_t tty - terminal to write to
// Return:	Nothing

void tty_writestr(char *string, size_t tty)
{
	tty_write(string, strlen(string), tty);
}




