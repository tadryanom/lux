
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
#include <tty.h>
#include <ustar.h>		// the only in-kernel FS

file_handle_t *files;
mountpoint_t *mountpoints;
char full_path[1024];
lock_t vfs_mutex = 0;
struct stat root_stat;

// vfs_init(): Initializes the virtual filesystem
// Param:	Nothing
// Return:	Nothing

void vfs_init()
{
	kprintf("vfs: initializing virtual filesystem...\n");
	files = kcalloc(sizeof(file_handle_t), MAX_FILES);
	mountpoints = kcalloc(sizeof(mountpoint_t), MAX_MOUNTPOINTS);

	// stat for root filesystem
	memset(&root_stat, 0, sizeof(struct stat));
	root_stat.st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	time_t timestamp = get_time();
	root_stat.st_atime = timestamp;
	root_stat.st_mtime = timestamp;
	root_stat.st_ctime = timestamp;

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
		if(path[1] == 0)
		{
			fullpath[0] = '/';
			fullpath[1] = 0;
			return 1;
		}

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

// open(): Opens a file
// Param:	const char *path - path of file
// Param:	int flags - open flags
// Return:	int - file handle

int open(const char *path, int flags, ...)
{
	struct stat file_info;
	int status = stat(path, &file_info);

	if(status != 0)
		return status;

	acquire_lock(&vfs_mutex);

	if(!file_info.st_mode & S_IFBLK || !file_info.st_mode & S_IFCHR || !file_info.st_mode & S_IFIFO || !file_info.st_mode & S_IFREG)
	{
		kprintf("vfs: can't open %s; it's not a file.\n");
		release_lock(&vfs_mutex);
		return ENOENT;
	}

	// resolve the path
	vfs_resolve_path(full_path, path);

	// check for the standard I/O stuff
	if(strcmp(full_path, "/dev/stdin") == 0)
	{
		release_lock(&vfs_mutex);
		return STDIN;
	} else if(strcmp(full_path, "/dev/stdout") == 0)
	{
		release_lock(&vfs_mutex);
		return STDOUT;
	} else if(strcmp(full_path, "/dev/stderr") == 0)
	{
		release_lock(&vfs_mutex);
		return STDERR;
	}

	// if not, we need to open the actual file
	// find an empty handle
	int handle = 0;

	while(files[handle].present != 0 && handle < MAX_FILES)
		handle++;

	if(handle >= MAX_FILES)
	{
		kprintf("vfs: no available file handles.\n");
		release_lock(&vfs_mutex);
		return ENOBUFS;
	}

	// create the file handle
	files[handle].present = 1;
	files[handle].position = 0;
	files[handle].flags = flags;
	files[handle].pid = get_pid();
	strcpy(files[handle].path, full_path);

	release_lock(&vfs_mutex);
	return handle;
}

// close(): Closes a file
// Param:	int handle - file handle
// Return:	int - status code

int close(int handle)
{
	if(files[handle].present != 1)
		return EBADF;

	acquire_lock(&vfs_mutex);
	memset(&files[handle], 0, sizeof(file_handle_t));
	release_lock(&vfs_mutex);
	return 0;
}

// read(): Reads a file
// Param:	int handle - file handle
// Param:	char *buffer - buffer to read
// Param:	size_t count - bytes to read
// Return:	ssize_t - bytes actually read

ssize_t read(int handle, char *buffer, size_t count)
{
	if(!count)
		return 0;

	// handle stdio stuff first
	if(handle == STDIN)
		return EIO;		// until I implement keyboard in userspace

	// can't read from the output
	if(handle == STDOUT || handle == STDERR)
		return EIO;

	acquire_lock(&vfs_mutex);
	if(files[handle].present != 1)
	{
		release_lock(&vfs_mutex);
		return EBADF;
	}

	if(memcmp(files[handle].path, "/dev/", 5) == 0)
		return devfs_read(handle, buffer, count);

	// for now
	release_lock(&vfs_mutex);
	return 0;
}

// write(): Writes a file
// Param:	int handle - file handle
// Param:	char *buffer - buffer to write
// Param:	size_t count - bytes to write
// Return:	size_t - bytes actually written

ssize_t write(int handle, char *buffer, size_t count)
{
	if(!count)
		return 0;

	// handle the stdio stuff here to be faster
	if(handle == STDOUT || handle == STDERR)
	{
		tty_write(buffer, count, get_tty());
		return count;
	}

	// can't write to the input
	if(handle == STDIN)
		return EIO;

	// if we get here, it's probably a real file
	acquire_lock(&vfs_mutex);
	if(files[handle].present != 1)
	{
		release_lock(&vfs_mutex);
		return EBADF;
	}

	if(memcmp(files[handle].path, "/dev/", 5) == 0)
		return devfs_write(handle, buffer, count);

	// for now
	release_lock(&vfs_mutex);
	return 0;
}

// lseek(): Moves the file pointer
// Param:	int handle - file handle
// Param:	off_t position - position of file
// Param:	int whence - position relativity
// Return:	int - new position or status code

int lseek(int handle, off_t position, int whence)
{
	if(files[handle].present != 1)
		return EBADF;

	struct stat file_info;
	int status = fstat(handle, &file_info);
	if(status != 0)
		return status;

	acquire_lock(&vfs_mutex);

	// for /dev files
	if(memcmp(files[handle].path, "/dev/", 5) == 0)
	{
		if(whence == SEEK_SET)
			files[handle].position = position;

		else if(whence == SEEK_CUR)
			files[handle].position += position;

		else if(whence == SEEK_END)
			files[handle].position = file_info.st_size - position;

		else
		{
			release_lock(&vfs_mutex);
			return EINVAL;
		}

		release_lock(&vfs_mutex);
		return files[handle].position;
	}

	// for other files
	if(whence == SEEK_SET)
	{
		if(position >= file_info.st_size)
		{
			release_lock(&vfs_mutex);
			return EINVAL;
		}

		files[handle].position = position;
		release_lock(&vfs_mutex);
		return files[handle].position;
	} else if(whence == SEEK_CUR)
	{
		if((files[handle].position + position) >= file_info.st_size)
		{
			release_lock(&vfs_mutex);
			return EINVAL;
		}

		files[handle].position += position;
		release_lock(&vfs_mutex);
		return files[handle].position;
	} else if(whence == SEEK_END)
	{
		if((file_info.st_size - position) >= file_info.st_size)
		{
			release_lock(&vfs_mutex);
			return EINVAL;
		}

		files[handle].position = file_info.st_size - position;
		release_lock(&vfs_mutex);
		return files[handle].position;
	} else
	{
		// undefined whence here
		release_lock(&vfs_mutex);
		return EINVAL;
	}
}

// stat(): Returns stat information for a file
// Param:	const char *path - path of file
// Param:	struct stat *destination - stat structure to store
// Return:	int - status code

int stat(const char *path, struct stat *destination)
{
	int status;

	acquire_lock(&vfs_mutex);

	vfs_resolve_path(full_path, path);
	if(memcmp(full_path, "/", 2) == 0)
	{
		memcpy(destination, &root_stat, sizeof(struct stat));
		release_lock(&vfs_mutex);
		return 0;
	}

	if(memcmp(full_path, "/dev", 5) == 0)
	{
		memcpy(destination, &devfs_stat, sizeof(struct stat));
		release_lock(&vfs_mutex);
		return 0;
	}

	if(memcmp(full_path, "/dev/", 5) == 0)
	{
		status = devstat(path + 5, destination);
		release_lock(&vfs_mutex);
		return status;
	}

	// determine the mountpoint, to call the proper filesystem driver
	int mountpoint = vfs_determine_mountpoint(full_path);
	if(mountpoint < 0)
	{
		release_lock(&vfs_mutex);
		return ENOENT;
	}

	char *tmp_path = kmalloc(1024);
	strcpy(tmp_path, full_path);
	release_lock(&vfs_mutex);

	if(strcmp(mountpoints[mountpoint].fstype, "ustar") == 0)
		status = ustar_stat(&mountpoints[mountpoint], tmp_path, destination);
	else
	{
		kprintf("vfs: undefined filesystem type: %s\n", mountpoints[mountpoint].fstype);
		status = ENOENT;
	}

	kfree(tmp_path);

	// TO-DO: Non-kernel filesystems will be added here
	// ext2 and FAT32 are intended for the foreseeable future
	return status;
}

// fstat(): Returns stat information for an open file
// Param:	int handle - file handle
// Param:	struct stat *destination - stat structure to store
// Return:	int - status code

int fstat(int handle, struct stat *destination)
{
	if(files[handle].present != 1)
		return EBADF;

	// normal stat()
	return stat(files[handle].path, destination);
}




