
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>

// kmalloc(): Allocates kernel memory
// Param:	size_t size - number of bytes to allocate
// Return:	void * - pointer to allocated memory, size_t aligned

void *kmalloc(size_t size)
{
	if(!size)
		return NULL;

	size_t pages = (size + sizeof(size_t) + PAGE_SIZE - 1) >> PAGE_SIZE_SHIFT;

	void *ptr = (void*)vmm_alloc(KERNEL_HEAP, pages, PAGE_PRESENT | PAGE_RW);
	if(!ptr)
		return NULL;

	size_t *header = (size_t*)ptr;
	header[0] = pages;		// store number of pages

	return ptr + sizeof(size_t);
}

// kcalloc(): Allocates kernel memory
// Param:	size_t size - size of each entry
// Param:	size_t count - number of entries
// Return:	void * - pointer to allocated memory, size_t aligned

void *kcalloc(size_t size, size_t count)
{
	return kmalloc(size * count);
}


