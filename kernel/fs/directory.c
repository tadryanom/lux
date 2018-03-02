
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
#include <lock.h>

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




