
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <vfs.h>
#include <time.h>

#define MAX_DEVFS_ENTRIES		512
#define DEVFS_MODE			(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

typedef struct devfs_entry_t
{
	char name[48];
	mode_t mode;
} devfs_entry_t;

void devfs_init();
void devfs_make_entry(char *, mode_t);




