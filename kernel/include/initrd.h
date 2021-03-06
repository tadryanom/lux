
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <boot.h>
#include <blkdev.h>

#define INITRD_SECTOR_SIZE		512

void initrd_init(multiboot_info_t *);
int initrd_read(blkdev_t *, uint64_t, uint64_t, void *);
int initrd_write(blkdev_t *, uint64_t, uint64_t, void *);



