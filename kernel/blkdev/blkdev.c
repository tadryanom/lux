
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <blkdev.h>
#include <vfs.h>
#include <mm.h>
#include <initrd.h>
#include <string.h>
#include <kprintf.h>

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
// Param:	uint16_t sector_size - size of sectors in bytes
// Param:	void *info - type-specific information, specific to the driver
// Param:	char *name - Name of the device
// Return:	dev_t - VFS-friendly number of the device

dev_t blkdev_register(uint8_t type, uint16_t sector_size, void *info, char *name)
{
	// find an unused block device slot
	dev_t device = 0;

	while(blkdevs[device].type != 0 && device < MAX_BLKDEVS)
		device++;

	if(device >= MAX_BLKDEVS)
		return 0;

	// and store everything there
	blkdevs[device].type = type;
	blkdevs[device].reserved = 0;
	blkdevs[device].sector_size = sector_size;

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

// blkdev_read(): Reads from a block device
// Param:	dev_t device - device to read from
// Param:	uint64_t lba - starting LBA sector
// Param:	uint64_t count - count of sectors to read
// Param:	void *buffer - buffer to read into
// Return:	int - return status

int blkdev_read(dev_t device, uint64_t lba, uint64_t count, void *buffer)
{
	if(!count)
		return 0;

	blkdev_t *blkdev = &blkdevs[device];

	if(blkdev->type == BLKDEV_INITRD)
		return initrd_read(blkdev, lba, count, buffer);

	kprintf("blkdev: read non-present device %d, LBA 0x%xq count %d\n", device, lba, count);
	return BLKDEV_NODEV;
}

// blkdev_read_bytes(): Reads from a block device using byte-indexing instead of sectors
// Param:	dev_t device - device to read from
// Param:	uint64_t base - starting byte
// Param:	uint64_t count - count of bytes to read
// Param:	void *buffer  buffer to read into
// Return:	int - return status

int blkdev_read_bytes(dev_t device, uint64_t base, uint64_t count, void *buffer)
{
	if(!count)
		return 0;

	blkdev_t *blkdev = &blkdevs[device];

	if(blkdev->type == 0 || blkdev->sector_size == 0)
		return BLKDEV_NODEV;

	uint64_t lba = base / blkdev->sector_size;	// round down
	uint64_t byte_start = base % blkdev->sector_size;
	uint64_t count_sectors = (count / blkdev->sector_size) + 1;

	void *tmp_buffer = kcalloc(blkdev->sector_size, count_sectors);
	int status = blkdev_read(device, lba, count_sectors, tmp_buffer);
	if(status != 0)
	{
		kfree(tmp_buffer);
		return status;
	}

	memcpy(buffer, tmp_buffer + byte_start, count);
	kfree(tmp_buffer);
	return 0;
}




