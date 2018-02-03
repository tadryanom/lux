
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <boot.h>
#include <mm.h>
#include <kprintf.h>
#include <screen.h>

uint16_t width, height, pitch;
uint16_t width_chars, height_chars, x_pos, y_pos;
uint32_t framebuffer;
size_t screen_size, screen_size_dwords;

// screen_init(): Initializes the screen
// Param:	vbe_mode_t *vbe_mode - VESA mode information
// Return:	Nothing

void screen_init(vbe_mode_t *vbe_info)
{
	kprintf("Using VESA framebuffer for output, %dx%dx%dbpp\n", vbe_info->width, vbe_info->height, vbe_info->bpp);
	kprintf("Framebuffer is at 0x%xd\n", vbe_info->framebuffer);

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

	// map the framebuffer
	vmm_map(FRAMEBUFFER, framebuffer, (screen_size / PAGE_SIZE) + 1, PAGE_PRESENT | PAGE_RW);
}



