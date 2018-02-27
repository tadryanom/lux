
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <kprintf.h>
#include <mm.h>
#include <blkdev.h>
#include <vfs.h>
#include <initrd.h>
#include <devfs.h>
#include <boot.h>
#include <string.h>

// initrd_init(): Detects the initial ramdisk
// Param:	multiboot_info_t *multiboot_info - pointer to multiboot information
// Return:	Nothing

void initrd_init(multiboot_info_t *multiboot_info)
{
	// check for modules
	if(!multiboot_info->flags & MULTIBOOT_FLAGS_MODULES || multiboot_info->mods_count == 0)
		panic("initrd not present, cannot use storage devices.");

	multiboot_module_t *module;
	module = (multiboot_module_t*)((size_t)multiboot_info->mods_addr);

	// register with the block device manager
	size_t size_pages = (module->mod_end - module->mod_start + (PAGE_SIZE-1)) / PAGE_SIZE;

	blkdev_initrd_t *initrd = kmalloc(sizeof(blkdev_initrd_t));
	initrd->size = sizeof(blkdev_initrd_t);
	initrd->base = (void*)vmm_request_map((size_t)module->mod_start, size_pages, PAGE_PRESENT | PAGE_RW);
	initrd->size_bytes = module->mod_end - module->mod_start;
	initrd->size_sectors = initrd->size_bytes / INITRD_SECTOR_SIZE;		// round down
	blkdev_register(BLKDEV_INITRD, INITRD_SECTOR_SIZE, initrd, "Initial ramdisk");

	kprintf("initrd: initrd is at 0x%xd, size 0x%xd bytes\n", module->mod_start, module->mod_end - module->mod_start);

#if __i386__
	kprintf("initrd: mapped at 0x%xd, %d pages\n", (size_t)initrd->base, size_pages);
#endif

#if __x86_64__
	kprintf("initrd: mapped at 0x%xq, %d pages\n", (size_t)initrd->base, size_pages);
#endif

	kfree(initrd);

	// register the initrd with the /dev filesystem
	devfs_make_entry("initrd", S_IFBLK | DEVFS_MODE);
}

// initrd_read(): Reads from the initrd
// Param:	blkdev_t *device - device to read
// Param:	uint64_t lba - starting LBA sector
// Param:	uint64_t count - count of sectors to read
// Param:	void *buffer - buffer to read into
// Return:	int - return status

int initrd_read(blkdev_t *device, uint64_t lba, uint64_t count, void *buffer)
{
	blkdev_initrd_t *initrd = (blkdev_initrd_t*)&device->data[0];
	if((lba + count) >= initrd->size_sectors)
		return BLKDEV_IO;

	memcpy(buffer, initrd->base + (lba * INITRD_SECTOR_SIZE), count * INITRD_SECTOR_SIZE);
	return 0;
}




