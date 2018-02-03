
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <mm.h>
#include <boot.h>
#include <kprintf.h>

// mm_init(): Initializes the memory manager
// Param:	multiboot_info_t *multiboot_info - pointer to multiboot information
// Return:	Nothing

void mm_init(multiboot_info_t *multiboot_info)
{
	pmm_init(multiboot_info);
	vmm_init();
}


