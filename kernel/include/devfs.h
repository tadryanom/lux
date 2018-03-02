
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
	struct stat information;
} devfs_entry_t;

struct stat devfs_stat;

void devfs_init();
void devfs_make_entry(char *, mode_t);
int devstat(const char *, struct stat *);
ssize_t devfs_read(int, char *, size_t);
ssize_t devfs_write(int, char *, size_t);



