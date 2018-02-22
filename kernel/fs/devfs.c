
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

// Implementation of /dev filesystem

devfs_entry_t *devfs_entries;
size_t devfs_count;
lock_t devfs_mutex = 0;

// devfs_init(): Initializes the /dev filesystem
// Param:	Nothing
// Return:	Nothing

void devfs_init()
{
	devfs_entries = kcalloc(sizeof(devfs_entry_t), MAX_DEVFS_ENTRIES);
	devfs_count = 0;

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
	devfs_entries[devfs_count].mode = mode;

	kprintf("devfs: registered device '%s'\n", name);
	devfs_count++;

	release_lock(&devfs_mutex);
}







