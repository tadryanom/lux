
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

size_t *pml4;
lock_t vmm_mutex = 0;
void vmm_map_page(size_t, size_t, uint8_t);

// vmm_init(): Initializes paging and the virtual memory manager
// Param:	Nothing
// Return:	Nothing

void vmm_init()
{
	// there's really nothing to do here --
	// -- because paging is always enabled in x86_64

	pml4 = (size_t*)(read_cr3() & (~(PAGE_SIZE-1)));
}

// vmm_get_page(): Returns physical address and flags of a page
// Param:	size_t page - 4KB-aligned page
// Return:	size_t - 4KB-aligned physical address and flags

size_t vmm_get_page(size_t page)
{
	// determine which PDPT has the page
	size_t pdpt = pml4[(page >> 39) & 511];		// 512 GB per each PML4 entry
	if((pdpt & PAGE_PRESENT) == 0)
		return 0;

	// now determine which page directory within the PDPT has the page
	pdpt &= (~(PAGE_SIZE-1));
	pdpt += PHYSICAL_MEMORY;

	size_t *pdpt_ptr = (size_t*)pdpt;

	size_t pdir = pdpt_ptr[(page >> 30) & 511];	// 1 GB per each PDPT entry
	if((pdir & PAGE_PRESENT) == 0)
		return 0;

	// now determine which page table within the page directory has the page
	pdir &= (~(PAGE_SIZE-1));
	pdir += PHYSICAL_MEMORY;

	size_t *pdir_ptr = (size_t*)pdir;

	size_t ptbl = pdir_ptr[(page >> 21) & 511];	// 2 MB per each page directory entry
	if((ptbl & PAGE_PRESENT) == 0)
		return 0;

	// for large pages, we don't actually have a page table to search
	if((ptbl & PAGE_LARGE) != 0)
		return ptbl;

	// determine which entry within the page table has the page
	ptbl &= (~(PAGE_SIZE-1));
	ptbl += PHYSICAL_MEMORY;

	size_t *ptbl_ptr = (size_t*)ptbl;

	return ptbl_ptr[(page >> PAGE_SIZE_SHIFT) & 511];
}

// vmm_map_page(): Maps a single page
// Param:	size_t virtual - virtual address
// Param:	size_t physical - physical address
// Param:	uint8_t flags - page flags
// Return:	Nothing

void vmm_map_page(size_t virtual, size_t physical, uint8_t flags)
{
	// determine which PDPT has the page
	size_t pdpt = pml4[(virtual >> 39) & 511];
	if((pdpt & PAGE_PRESENT) == 0)
	{
		// PDPT doesn't exist, make a PDPT
		pdpt = pmm_alloc(1);
		memset((void*)(pdpt + PHYSICAL_MEMORY), 0, 4096);
		pml4[(virtual >> 39) & 511] = pdpt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
	}

	// determine which page directory has the page
	pdpt &= (~(PAGE_SIZE-1));
	pdpt += PHYSICAL_MEMORY;
	size_t *pdpt_ptr = (size_t*)pdpt;

	size_t pdir = pdpt_ptr[(virtual >> 30) & 511];
	if((pdir & PAGE_PRESENT) == 0)
	{
		// page directory doesn't exist, make a page directory
		pdir = pmm_alloc(1);
		memset((void*)(pdir + PHYSICAL_MEMORY), 0, 4096);
		pdpt_ptr[(virtual >> 30) & 511] = pdir | PAGE_PRESENT | PAGE_RW | PAGE_USER;
	}

	// determine which page table has the page
	pdir &= (~(PAGE_SIZE-1));
	pdir += PHYSICAL_MEMORY;
	size_t *pdir_ptr = (size_t*)pdir;

	size_t ptbl = pdir_ptr[(virtual >> 21) & 511];
	if((ptbl & PAGE_PRESENT) == 0)
	{
		// page table doesn't exist, make a page table
		ptbl = pmm_alloc(1);
		memset((void*)(ptbl + PHYSICAL_MEMORY), 0, 4096);
		pdir_ptr[(virtual >> 21) & 511] = ptbl | PAGE_PRESENT | PAGE_RW | PAGE_USER;
	}

	// map the page
	ptbl &= (~(PAGE_SIZE-1));
	ptbl += PHYSICAL_MEMORY;

	size_t *ptbl_ptr = (size_t*)ptbl;
	ptbl_ptr[(virtual >> PAGE_SIZE_SHIFT) & 511] = physical | flags;
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
		vmm_map_page(virtual + (i << PAGE_SIZE_SHIFT), physical + (i << PAGE_SIZE_SHIFT), flags);
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
	vmm_map(virtual, 0, count, 0);
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
	acquire_lock(&vmm_mutex);

	if(!count)
	{
		release_lock(&vmm_mutex);
		return NULL;
	}

	// allocate virtual memory
	size_t virtual = vmm_find_range(start, count);
	if(!virtual)
	{
		release_lock(&vmm_mutex);
		return NULL;
	}

	// and physical memory
	size_t physical = pmm_alloc(count);
	if(!physical)
	{
		release_lock(&vmm_mutex);
		return NULL;
	}

	vmm_map(virtual, physical, count, flags);

	// zero-initialize
	memset((void*)virtual, 0, count << PAGE_SIZE_SHIFT);
	release_lock(&vmm_mutex);
	return virtual;
}

// vmm_free(): Frees memory
// Param:	size_t ptr - pointer to memory
// Param:	size_t count - count of pages
// Return:	Nothing

void vmm_free(size_t ptr, size_t count)
{
	acquire_lock(&vmm_mutex);
	if(!count)
	{
		release_lock(&vmm_mutex);
		return;
	}

	size_t physical = vmm_get_page(ptr);
	if(!physical & PAGE_PRESENT)		// page not present?
	{
		release_lock(&vmm_mutex);
		return;
	}

	physical &= (~(PAGE_SIZE-1));

	pmm_mark_free(physical, count);
	vmm_unmap(ptr, count);	
	release_lock(&vmm_mutex);
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

	if(flags == (PAGE_PRESENT | PAGE_RW) || flags == PAGE_RW)
		return physical + PHYSICAL_MEMORY;

	acquire_lock(&vmm_mutex);

	// allocate virtual memory
	count++;
	size_t virtual = vmm_find_range(KERNEL_HEAP, count);
	if(!virtual)
	{
		release_lock(&vmm_mutex);
		return NULL;
	}

	vmm_map(virtual, physical, count, flags);
	release_lock(&vmm_mutex);
	return virtual + (physical & (PAGE_SIZE-1));
}

#endif			// __x86_64__





