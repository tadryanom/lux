
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <vfs.h>
#include <mm.h>
#include <kprintf.h>
#include <tasking.h>
#include <string.h>
#include <devfs.h>

file_handle_t *files;
mountpoint_t *mountpoints;
char full_path[1024];

// vfs_init(): Initializes the virtual filesystem
// Param:	Nothing
// Return:	Nothing

void vfs_init()
{
	kprintf("vfs: initializing virtual filesystem...\n");
	files = kcalloc(sizeof(file_handle_t), MAX_FILES);
	mountpoints = kcalloc(sizeof(mountpoint_t), MAX_MOUNTPOINTS);

	devfs_init();

	// mark the first three file handles as used, for stdin, stdout, stderr
	files[STDIN].present = 1;
	strcpy(files[STDIN].path, "/dev/stdin");

	files[STDOUT].present = 1;
	strcpy(files[STDOUT].path, "/dev/stdout");

	files[STDERR].present = 1;
	strcpy(files[STDERR].path, "/dev/stderr");
}

// dir_open(): Opens a directory
// Param:	char *path - path of directory
// Return:	directory_t * - pointer to directory handle, NULL on error

directory_t *dir_open(char *path)
{
	return NULL;
}

// dir_close(): Closes a directory
// Param:	directory_t *directory - pointer to directory handle
// Return:	Nothing

void dir_close(directory_t *directory)
{
	kfree(directory);
}

// dir_query(): Queries a directory
// Param:	directory_t *directory - pointer to directory handle
// Param:	directory_entry_t *entry - pointer to entry to store
// Return:	int - 0 on success

int dir_query(directory_t *directory, directory_entry_t *entry)
{
	return -1;
}

// open(): Opens a file
// Param:	const char *path - path of file
// Param:	int flags - open flags
// Return:	int - file handle

int open(const char *path, int flags, ...)
{
	return -1;
}






