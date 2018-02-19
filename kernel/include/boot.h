
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define MULTIBOOT_MAGIC			0x2BADB002
#define MULTIBOOT_FLAGS_MEMORY		0x00000001
#define MULTIBOOT_FLAGS_BOOT_DEVICE	0x00000002
#define MULTIBOOT_FLAGS_CMDLINE		0x00000004
#define MULTIBOOT_FLAGS_MODULES		0x00000008
#define MULTIBOOT_FLAGS_MMAP		0x00000040

#define E820_USABLE			1
#define E820_RESERVED			2
#define E820_ACPI_DATA			3
#define E820_ACPI_NVS			4
#define E820_BAD			5

typedef struct multiboot_info_t
{
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint8_t boot_device[4];
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms[4];
	uint32_t mmap_length;
	uint32_t mmap_addr;
}__attribute__((packed)) multiboot_info_t;

typedef struct e820_entry_t
{
	uint32_t size;
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t acpi_attributes;		// for ACPI 3.0
}__attribute__((packed)) e820_entry_t;

typedef struct vbe_mode_t
{
	uint16_t attributes;
	uint8_t window_a;
	uint8_t window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;

	uint16_t pitch;
	uint16_t width;
	uint16_t height;

	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;
	uint8_t bpp;
	uint8_t banks;

	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved0;

	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t reserved_mask;
	uint8_t direct_color;

	uint32_t framebuffer;
	uint32_t off_screen_mem;
	uint32_t off_screen_mem_size;

	uint8_t reserved1[206];
}__attribute__((packed)) vbe_mode_t;




