
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <vfs.h>
#include <mm.h>
#include <string.h>
#include <kprintf.h>
#include <lock.h>
#include <devfs.h>
#include <time.h>

// Implementation of /dev filesystem

devfs_entry_t *devfs_entries;
size_t devfs_count;
lock_t devfs_mutex = 0;
struct stat devfs_stat;

// devfs_init(): Initializes the /dev filesystem
// Param:	Nothing
// Return:	Nothing

void devfs_init()
{
	devfs_entries = kcalloc(sizeof(devfs_entry_t), MAX_DEVFS_ENTRIES);
	devfs_count = 0;

	time_t timestamp = get_time();
	devfs_stat.st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	devfs_stat.st_atime = timestamp;
	devfs_stat.st_mtime = timestamp;
	devfs_stat.st_ctime = timestamp;

	devfs_make_entry("null", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("zero", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stdin", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stdout", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stderr", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("vesafb", S_IFCHR | DEVFS_MODE);
}

// devfs_make_entry(): Makes an entry in the /dev filesystem
// Param:	char *name - name of entry
// Param:	mode_t mode - mode of entry
// Return:	Nothing

void devfs_make_entry(char *name, mode_t mode)
{
	acquire_lock(&devfs_mutex);
	strcpy(devfs_entries[devfs_count].name, name);
	devfs_entries[devfs_count].information.st_mode = mode;
	devfs_entries[devfs_count].information.st_size = sizeof(size_t);

	time_t timestamp = get_time();
	devfs_entries[devfs_count].information.st_atime = timestamp;
	devfs_entries[devfs_count].information.st_mtime = timestamp;
	devfs_entries[devfs_count].information.st_ctime = timestamp;

	kprintf("devfs: registered device '%s'\n", name);
	devfs_count++;

	release_lock(&devfs_mutex);
}

// devstat(): Returns stat information for a /dev node
// Param:	const char *name - name of node
// Param:	struct stat *destination - structure to store information
// Return:	int - return status

int devstat(const char *name, struct stat *destination)
{
	size_t entry = 0;
	while(memcmp(name, devfs_entries[entry].name, strlen(name) + 1) != 0)
	{
		entry++;
		if(entry >= MAX_DEVFS_ENTRIES)
			return ENOENT;
	}

	memcpy(destination, &devfs_entries[entry].information, sizeof(struct stat));

	// update access time
	devfs_entries[devfs_count].information.st_atime = get_time();
	return 0;
}





