
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>
#include <boot.h>
#include <kprintf.h>
#include <string.h>
#include <lock.h>

#if __x86_64__

// pmm_init(): Initializes the physical memory manager
// Param:	multiboot_info_t *multiboot_info - pointer to multiboot information
// Return:	Nothing

void pmm_init(multiboot_info_t *multiboot_info)
{
}

// pmm_add_range(): Adds a memory range to the physical memory manager
// Param:	e820_entry_t *mmap - pointer to memory range structure
// Return:	Nothing

void pmm_add_range(e820_entry_t *mmap)
{
}

// pmm_mark_page_used(): Marks a single page as used
// Param:	size_t page - 4KB-aligned page address
// Return:	Nothing

void pmm_mark_page_used(size_t page)
{
}

// pmm_mark_page_free(): Marks a single page as free
// Param:	size_t page - 4KB-aligned page address
// Return:	Nothing

void pmm_mark_page_free(size_t page)
{
}

// pmm_mark_used(): Marks a range of pages as used
// Param:	size_t base - 4KB-aligned base
// Param:	size_t count - count of pages
// Return:	Nothing

void pmm_mark_used(size_t base, size_t count)
{
}

// pmm_mark_free(): Marks a range of pages as free
// Param:	size_t base - 4KB-aligned base
// Param:	size_t count - count of pages
// Return:	Nothing

void pmm_mark_free(size_t base, size_t count)
{
}

// pmm_is_page_free(): Checks if a page is free or used
// Param:	size_t page - 4KB-aligned page
// Return:	uint8_t - 1 for used pages, 0 for free pages

uint8_t pmm_is_page_free(size_t page)
{
	return 1;
}

// pmm_find_range(): Finds a range of contiguous physical pages
// Param:	size_t count - count of pages
// Return:	size_t - start of 4KB-aligned page, NULL on error

size_t pmm_find_range(size_t count)
{
	return NULL;
}

// pmm_alloc(): Allocates contiguous physical pages
// Param:	size_t count - count of pages
// Return:	size_t - start of 4KB-aligned page, NULL on error

size_t pmm_alloc(size_t count)
{
	return NULL;
}

#endif		// __x86_64__




