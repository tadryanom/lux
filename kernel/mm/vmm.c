
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>
#include <kprintf.h>
#include <cpu.h>
#include <string.h>

size_t *page_directory, *page_tables;

// vmm_init(): Initializes paging and the virtual memory manager
// Param:	Nothing
// Return:	Nothing

void vmm_init()
{
	size_t tmp_ptr;

	tmp_ptr = (size_t)(pmm_bitmap + PMM_BITMAP_SIZE + (PAGE_SIZE - 1)) & (~(PAGE_SIZE-1));
	page_directory = (size_t*)tmp_ptr;

	tmp_ptr += PAGE_SIZE;
	page_tables = (size_t*)tmp_ptr;

	// create the page directory
	size_t i = 0;

	while(i < 1024)
	{
		page_directory[i] = (size_t)page_tables + (i << PAGE_SIZE_SHIFT) + PAGE_PRESENT | PAGE_RW | PAGE_USER;
		i++;
	}

	// clear the page tables
	i = 0;
	while(i < 1024*1024)
	{
		page_tables[i] = 0;
		i++;
	}

	// identity-map the lowest 16 MB
	i = 0;
	while(i < 4096)
	{
		page_tables[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
		i++;
	}

	// enable paging
	write_cr3((uint32_t)page_directory);
	uint32_t cr0 = read_cr0();
	cr0 |= 0x80000000;
	cr0 &= ~0x60000000;		// caching
	write_cr0(cr0);
}

// vmm_get_page(): Returns physical address and flags of a page
// Param:	size_t page - 4KB-aligned page
// Return:	size_t - 4KB-aligned physical address and flags

inline size_t vmm_get_page(size_t page)
{
	return page_tables[page >> PAGE_SIZE_SHIFT];
}

// vmm_map(): Maps physical memory in the virtual address space
// Param:	size_t virtual - start of virtual base
// Param:	size_t physical - start of physical base
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	Nothing

void vmm_map(size_t virtual, size_t physical, size_t count, uint8_t flags)
{
	if(!count)
		return;

	size_t i = 0;
	while(i < count)
	{
		page_tables[(virtual >> PAGE_SIZE_SHIFT) + i] = (physical + (i << PAGE_SIZE_SHIFT)) | (size_t)flags;
		i++;
	}

	flush_tlb(virtual, count);
}

// vmm_unmap(): Unmaps memory from the virtual address space
// Param:	size_t virtual - start of virtual base
// Param:	size_t count - count of pages
// Return:	Nothing

void vmm_unmap(size_t virtual, size_t count)
{
	if(!count)
		return;

	vmm_map(virtual, 0, count, 0);
	//flush_tlb(virtual, count);
}

// vmm_find_range(): Finds a range of free pages
// Param:	size_t start - start of virtual base
// Param:	size_t count - count of pages
// Return:	size_t - Pointer to first free page

size_t vmm_find_range(size_t start, size_t count)
{
	if(!count)
		return NULL;

	size_t current_return = start;
	size_t free_count = 0;

	while(free_count < count)
	{
		if(!vmm_get_page(current_return + (free_count << PAGE_SIZE_SHIFT)) & PAGE_PRESENT)
			free_count++;

		else
		{
			current_return += PAGE_SIZE;

			if(current_return >= (~0) - PAGE_SIZE - (count << PAGE_SIZE_SHIFT))
				return NULL;

			free_count = 0;
		}
	}

	return current_return;
}

// vmm_alloc(): Allocates memory
// Param:	size_t start - start of virtual base
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	size_t - Pointer to allocated memory

size_t vmm_alloc(size_t start, size_t count, uint8_t flags)
{
	if(!count)
		return NULL;

	// allocate virtual memory
	size_t virtual = vmm_find_range(start, count);
	if(!virtual)
		return NULL;

	// and physical memory
	size_t physical = pmm_alloc(count);
	if(!physical)
		return NULL;

	vmm_map(virtual, physical, count, flags);

	// zero-initialize
	memset((void*)virtual, 0, count << PAGE_SIZE_SHIFT);
	return virtual;
}

// vmm_free(): Frees memory
// Param:	size_t ptr - pointer to memory
// Param:	size_t count - count of pages
// Return:	Nothing

void vmm_free(size_t ptr, size_t count)
{
	if(!count)
		return;

	size_t physical = vmm_get_page(ptr);
	if(!physical & PAGE_PRESENT)		// page not present?
		return;

	physical &= (~(PAGE_SIZE-1));

	pmm_mark_free(physical, count);
	vmm_unmap(ptr, count);	
}

// vmm_request_map(): Requests physical memory be mapped
// Param:	size_t physical - physical address
// Param:	size_t count - count of pages
// Param:	uint8_t flags - page flags
// Return:	size_t - virtual address

size_t vmm_request_map(size_t physical, size_t count, uint8_t flags)
{
	if(!count)
		return NULL;

	// allocate virtual memory
	count++;
	size_t virtual = vmm_find_range(KERNEL_HEAP, count);
	if(!virtual)
		return NULL;

	vmm_map(virtual, physical, count, flags);

	return virtual + (physical & (PAGE_SIZE-1));
}




