
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>
#include <boot.h>
#include <kprintf.h>
#include <string.h>
#include <lock.h>

#if __i386__

uint8_t *pmm_bitmap;
size_t total_pages, used_pages, reserved_pages;
uint64_t total_memory, usable_memory;
size_t highest_usable_address;
lock_t pmm_mutex = 0;

void pmm_add_range(e820_entry_t *);
void pmm_mark_page_used(size_t);
void pmm_mark_page_free(size_t);

// pmm_init(): Initializes the physical memory manager
// Param:	multiboot_info_t *multiboot_info - pointer to multiboot information
// Return:	Nothing

void pmm_init(multiboot_info_t *multiboot_info)
{
	char hex_string[17];

	if(!multiboot_info->flags & MULTIBOOT_FLAGS_MMAP || !multiboot_info->mmap_length || !multiboot_info->mmap_addr)
	{
		kprintf("boot error: E820 memory map is not present.\n");
		while(1);
	}

	// we have to have at least 32 MB contiguous
	if(multiboot_info->mem_upper < 32768)
	{
		kprintf("boot error: too little memory present.\n");
		while(1);
	}

	// create a bitmap at the end of the kernel binary
	pmm_bitmap = (uint8_t*)(&kend);
	memset(pmm_bitmap, 0, PMM_BITMAP_SIZE);

	// and start!
	total_pages = 0;
	used_pages = 0;
	reserved_pages = 0;
	total_memory = 0;
	usable_memory = 0;

	kprintf("pmm: showing E820 memory map:\n");
	kprintf(" STARTING ADDRESS - ENDING ADDRESS   - TYPE\n");

	e820_entry_t *mmap = (e820_entry_t*)(multiboot_info->mmap_addr);
	e820_entry_t *mmap_end = (e820_entry_t*)(multiboot_info->mmap_addr + multiboot_info->mmap_length);

	while(mmap < mmap_end)
	{
		kprintf(" %xq - %xq - ", mmap->base, mmap->base + mmap->length);

		switch(mmap->type)
		{
		case E820_USABLE:
			kprintf("usable RAM");
			break;

		case E820_RESERVED:
			kprintf("hardware-reserved");
			break;

		case E820_ACPI_DATA:
			kprintf("ACPI data");
			break;

		case E820_ACPI_NVS:
			kprintf("ACPI NVS");
			break;

		case E820_BAD:
			kprintf("bad memory");
			break;

		default:
			kprintf("undefined type");
			break;
		}

		kprintf("\n");

		// add to the list
		pmm_add_range(mmap);

		// go on...
		mmap = (e820_entry_t*)((uint32_t)mmap + mmap->size + 4);
	}

	kprintf("pmm: total of %d MB memory, of which %d MB are usable.\n", (uint32_t)total_memory/ 1024/1024, (uint32_t)usable_memory/1024/1024);

	// mark the lowest 16 MB for the kernel
	pmm_mark_used(0, 4096);
	kprintf("pmm: %d pages, %d used, %d hardware reserved.\n", total_pages, used_pages, reserved_pages);
}

// pmm_add_range(): Adds a memory range to the physical memory manager
// Param:	e820_entry_t *mmap - pointer to memory range structure
// Return:	Nothing

void pmm_add_range(e820_entry_t *mmap)
{
	// ignore above 4 GB because we don't use PAE
	if(mmap->base >= 0x100000000 || mmap->base + mmap->length >= 0x100000000)
		return;

	if(!mmap->length)		// zero-size entry?
		return;			// ignore

	// check for ACPI 3.0
	if(mmap->size >= 24)
	{
		if(!mmap->acpi_attributes & 1)		// ignore entry?
			return;				// -- yep
	}

	// add to the count of pages
	total_pages += (mmap->length + PAGE_SIZE-1) / PAGE_SIZE;
	total_memory += mmap->length;

	if(mmap->type == E820_USABLE)
	{
		usable_memory += mmap->length;
		highest_usable_address = (size_t)mmap->base + mmap->length - (PAGE_SIZE-1);
	} else
	{
		reserved_pages += (mmap->length + PAGE_SIZE-1) / PAGE_SIZE;
		pmm_mark_used((size_t)mmap->base, (size_t)(mmap->length + PAGE_SIZE-1) / PAGE_SIZE);
	}
}

/* ******************************** */
/*                                  */
/* Actual management functions here */
/*                                  */
/* ******************************** */

// pmm_mark_page_used(): Marks a single page as used
// Param:	size_t page - 4KB-aligned page address
// Return:	Nothing

void pmm_mark_page_used(size_t page)
{
	if(pmm_bitmap[page >> PAGE_SIZE_SHIFT] == 1)
		return;

	pmm_bitmap[page >> PAGE_SIZE_SHIFT] = 1;
	used_pages++;
}

// pmm_mark_page_free(): Marks a single page as free
// Param:	size_t page - 4KB-aligned page address
// Return:	Nothing

void pmm_mark_page_free(size_t page)
{
	if(pmm_bitmap[page >> PAGE_SIZE_SHIFT] == 0)
		return;

	pmm_bitmap[page >> PAGE_SIZE_SHIFT] = 0;
	used_pages--;
}

// pmm_mark_used(): Marks a range of pages as used
// Param:	size_t base - 4KB-aligned base
// Param:	size_t count - count of pages
// Return:	Nothing

void pmm_mark_used(size_t base, size_t count)
{
	if(!count)
		return;

	size_t i = 0;
	while(i < count)
	{
		pmm_mark_page_used(base);
		i++;
		base += PAGE_SIZE;
	}
}

// pmm_mark_free(): Marks a range of pages as free
// Param:	size_t base - 4KB-aligned base
// Param:	size_t count - count of pages
// Return:	Nothing

void pmm_mark_free(size_t base, size_t count)
{
	if(!count)
		return;

	size_t i = 0;
	while(i < count)
	{
		pmm_mark_page_free(base);
		i++;
		base += PAGE_SIZE;
	}
}

// pmm_is_page_free(): Checks if a page is free or used
// Param:	size_t page - 4KB-aligned page
// Return:	uint8_t - 1 for used pages, 0 for free pages

uint8_t pmm_is_page_free(size_t page)
{
	if(page >= highest_usable_address)
		return 1;

	else
		return pmm_bitmap[page >> PAGE_SIZE_SHIFT];
}

// pmm_find_range(): Finds a range of contiguous physical pages
// Param:	size_t count - count of pages
// Return:	size_t - start of 4KB-aligned page, NULL on error

size_t pmm_find_range(size_t count)
{
	if(!count)
		return NULL;

	// we have reserved the lowest 16 MB for the kernel
	// so start looking from 16 MB
	size_t current_return = 0x1000000;
	size_t free_count = 0;

	while(free_count < count)
	{
		if(pmm_is_page_free(current_return + (free_count << PAGE_SIZE_SHIFT)) == 0)
			free_count++;

		else
		{
			current_return += PAGE_SIZE;

			if(current_return >= highest_usable_address)
				return NULL;

			free_count = 0;
		}
	}

	return current_return;
}

// pmm_alloc(): Allocates contiguous physical pages
// Param:	size_t count - count of pages
// Return:	size_t - start of 4KB-aligned page, NULL on error

size_t pmm_alloc(size_t count)
{
	acquire_lock(&pmm_mutex);

	size_t memory = pmm_find_range(count);
	if(!memory)
	{
		release_lock(&pmm_mutex);
		return NULL;
	}

	pmm_mark_used(memory, count);
	release_lock(&pmm_mutex);
	return memory;
}

#endif // __i386__



