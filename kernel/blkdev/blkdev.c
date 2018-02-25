
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <blkdev.h>
#include <vfs.h>
#include <mm.h>
#include <initrd.h>
#include <string.h>

blkdev_t *blkdevs;
size_t blkdev_count = 0;

// blkdev_init(): Initializes block devices
// Param:	multiboot_info_t *multiboot_info - pointer to multiboot information
// Return:	Nothing

void blkdev_init(multiboot_info_t *multiboot_info)
{
	blkdevs = kcalloc(sizeof(blkdev_t), MAX_BLKDEVS);
	initrd_init(multiboot_info);
}

// blkdev_register(): Registers a block device
// Param:	uint8_t type - type of block device
// Param:	void *info - type-specific information, specific to the driver
// Param:	char *name - Name of the device
// Return:	dev_t - VFS-friendly number of the device

dev_t blkdev_register(uint8_t type, void *info, char *name)
{
	// find an unused block device slot
	dev_t device = 0;

	while(blkdevs[device].type != 0)
		device++;

	// and store everything there
	blkdevs[device].type = type;

	uint16_t *info_size = (uint16_t*)info;
	memcpy(blkdevs[device].data, info, (size_t)info_size[0] & 0xFFFF);

	if(strlen(name) > 63)
		memcpy(blkdevs[device].name, name, 63);
	else
		strcpy(blkdevs[device].name, name);

	blkdevs[device].name[63] = 0;

	blkdev_count++;
	return device;
}


