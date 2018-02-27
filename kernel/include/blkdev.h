
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <boot.h>
#include <vfs.h>

#define MAX_BLKDEVS		256

// Error codes
#define BLKDEV_NODEV		1
#define BLKDEV_IO		2

// Only an INITRD driver is built-in to the kernel
// ATA, AHCI and other stuff will be in external modules
#define BLKDEV_NONE		0
#define BLKDEV_INITRD		1

typedef struct blkdev_t
{
	uint8_t type;		// type of device as constants above
	uint8_t reserved;
	uint16_t sector_size;
	uint8_t data[188];	// type-specific data
	char name[64];		// name of device
} blkdev_t;

typedef struct blkdev_initrd_t
{
	uint16_t size;		// total size of this specific structure
	void *base;
	uint32_t size_bytes;
	uint32_t size_sectors;
} blkdev_initrd_t;

blkdev_t *blkdevs;
size_t blkdev_count;

void blkdev_init(multiboot_info_t *);
dev_t blkdev_register(uint8_t, uint16_t, void *, char *);
int blkdev_read(dev_t, uint64_t, uint64_t, void *);
int blkdev_write(dev_t, uint64_t, uint64_t, void *);
int blkdev_read_bytes(dev_t, uint64_t, uint64_t, void *);
int blkdev_write_bytes(dev_t, uint64_t, uint64_t, void *);




