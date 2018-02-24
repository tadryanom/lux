
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

// vfs_resolve_path(): Resolves a path
// Param:	char *fullpath - destination
// Param:	char *path - source
// Return:	size_t - size of final string

size_t vfs_resolve_path(char *fullpath, const char *path)
{
	size_t i = 0, j = 0;

	if(path[0] == '/')
	{
		// starting from the root directory here
		fullpath[0] = '/';

		i = 1;
		j = 1;
	} else
	{
	}

	while(path[i] != 0)
	{
		if(path[i] == '.' && path[i+1] == '/')		// handle '.' = current dir
			i += 2;

		// and handle '..' = previous dir
		else if(path[i] == '.' && path[i+1] == '.' && path[i+2] == '/')
		{
			i += 3;
			if(j > 1)
			{
				while(fullpath[j] != '/')
					j--;

				j--;
				while(fullpath[j] != '/')
					j--;

				j++;
				fullpath[j] = 0;
			}
		}

		else
		{
			fullpath[j] = path[i];
			j++;
			i++;
		}
	}

	while(fullpath[j-1] == '/')
	{
		fullpath[j-1] = 0;
		j--;
	}

	fullpath[j] = 0;
	return strlen(fullpath);
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

// stat(): Returns stat information for a file
// Param:	const char *path - path of file
// Param:	struct stat *destination - stat structure to store
// Return:	int - status code

int stat(const char *path, struct stat *destination)
{
	vfs_resolve_path(full_path, path);
	if(memcmp(full_path, "/dev", 5) == 0)
	{
		memcpy(destination, &devfs_stat, sizeof(struct stat));
		return 0;
	}

	if(memcmp(full_path, "/dev/", 5) == 0)
		return devstat(path + 5, destination);

	return ENOENT;		// for now
}





