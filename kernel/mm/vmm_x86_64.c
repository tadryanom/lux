
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>
#include <kprintf.h>
#include <cpu.h>
#include <string.h>
#include <lock.h>

#if __x86_64__

// vmm_init(): Initializes paging and the virtual memory manager
// Param:	Nothing
// Return:	Nothing

void vmm_init()
{
}

// vmm_get_page(): Returns physical address and flags of a page
// Param:	size_t page - 4KB-aligned page
// Return:	size_t - 4KB-aligned physical address and flags

size_t vmm_get_page(size_t page)
{
	return 0;
}

// vmm_map(): Maps physical memory in the virtual address space
// Param:	size_t virtual - start of virtual base
// Param:	size_t physical - start of physical base
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	Nothing

void vmm_map(size_t virtual, size_t physical, size_t count, uint8_t flags)
{
}

// vmm_unmap(): Unmaps memory from the virtual address space
// Param:	size_t virtual - start of virtual base
// Param:	size_t count - count of pages
// Return:	Nothing

void vmm_unmap(size_t virtual, size_t count)
{
}

// vmm_find_range(): Finds a range of free pages
// Param:	size_t start - start of virtual base
// Param:	size_t count - count of pages
// Return:	size_t - Pointer to first free page

size_t vmm_find_range(size_t start, size_t count)
{
	return NULL;
}

// vmm_alloc(): Allocates memory
// Param:	size_t start - start of virtual base
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	size_t - Pointer to allocated memory

size_t vmm_alloc(size_t start, size_t count, uint8_t flags)
{
	return NULL;
}

// vmm_free(): Frees memory
// Param:	size_t ptr - pointer to memory
// Param:	size_t count - count of pages
// Return:	Nothing

void vmm_free(size_t ptr, size_t count)
{
}

// vmm_request_map(): Requests physical memory be mapped
// Param:	size_t physical - physical address
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	size_t - virtual address

size_t vmm_request_map(size_t physical, size_t count, uint8_t flags)
{
	return NULL;
}

#endif			// __x86_64__





