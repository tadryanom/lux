
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
#include <blkdev.h>
#include <rand.h>
#include <tty.h>
#include <tasking.h>
#include <io.h>

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

	memset(&devfs_stat, 0, sizeof(struct stat));
	time_t timestamp = get_time();
	devfs_stat.st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	devfs_stat.st_atime = timestamp;
	devfs_stat.st_mtime = timestamp;
	devfs_stat.st_ctime = timestamp;

	// these devices are always here
	devfs_make_entry("null", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("zero", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stdin", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stdout", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("stderr", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("vesafb", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("random", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("urandom", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("port", S_IFCHR | DEVFS_MODE);
	devfs_make_entry("tty", S_IFCHR | DEVFS_MODE);

	// register tty terminals
	size_t tty = 0;
	char tty_name[8];
	while(tty < TTY_COUNT && tty < 10)
	{
		sprintf(tty_name, "tty%d", tty);
		devfs_make_entry(tty_name, S_IFCHR | DEVFS_MODE);
		tty++;
	}
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

// devfs_read(): Reads from a file on /dev
// Param:	int handle - file handle
// Param:	char *buffer - buffer to read
// Param:	size_t count - bytes to read
// Return:	int - bytes actually read, on error code

ssize_t devfs_read(int handle, char *buffer, size_t count)
{
	int blkdev_status;
	uint64_t blkdev_base;
	size_t random_count = 0;
	uint8_t *byte;
	uint16_t *word;
	uint32_t *dword;

	if(strcmp(files[handle].path, "/dev/vesafb") == 0)
	{
		void *framebuffer = (void*)HW_FRAMEBUFFER + files[handle].position;
		memcpy(buffer, framebuffer, count);

		files[handle].position += count;
		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/initrd") == 0)
	{
		blkdev_base = (uint64_t)files[handle].position;
		blkdev_status = blkdev_read_bytes(0, blkdev_base, count, buffer);
		release_lock(&vfs_mutex);

		if(blkdev_status == 0)
			return count;
		else
			return EIO;
	} else if(strcmp(files[handle].path, "/dev/zero") == 0 || strcmp(files[handle].path, "/dev/null") == 0)
	{
		// simply put zeroes
		memset(buffer, 0, count);
		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/random") == 0 || strcmp(files[handle].path, "/dev/urandom") == 0)
	{
		// random numbers here
		while(random_count < count)
		{
			buffer[random_count] = (char)rand() & 0xFF;
			random_count++;
		}

		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/port") == 0)
	{
		// read from I/O port here
		if(count == 1)
		{
			byte = (uint8_t*)buffer;
			byte[0] = inb((uint16_t)files[handle].position);
		} else if(count == 2)
		{
			word = (uint16_t*)buffer;
			word[0] = inw((uint16_t)files[handle].position);
		} else if(count == 4)
		{
			dword = (uint32_t*)buffer;
			dword[0] = ind((uint16_t)files[handle].position);
		} else
		{
			kprintf("devfs: attempted to read undefined size %d from I/O port 0x%xw\n", count, (uint16_t)files[handle].position);
			release_lock(&vfs_mutex);
			return EIO;
		}

		release_lock(&vfs_mutex);
		return count;
	}

	release_lock(&vfs_mutex);
	return 0;
}

// devfs_write(): Writes to a file on /dev
// Param:	int handle - file handle
// Param:	char *buffer - buffer to write
// Param:	size_t count - byte count
// Return:	int - bytes actually written, or error code

ssize_t devfs_write(int handle, char *buffer, size_t count)
{
	uint8_t *byte;
	uint16_t *word;
	uint32_t *dword;

	// handle framebuffer first for graphics performance later on
	if(strcmp(files[handle].path, "/dev/vesafb") == 0)
	{
		void *framebuffer = (void*)HW_FRAMEBUFFER + files[handle].position;
		memcpy(framebuffer, buffer, count);

		files[handle].position += count;
		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/zero") == 0 || strcmp(files[handle].path, "/dev/null") == 0)
	{
		// don't do anything, but return success
		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/tty") == 0)
	{
		tty_write(buffer, count, get_tty());
		release_lock(&vfs_mutex);
		return count;
	} else if(memcmp(files[handle].path, "/dev/tty", 8) == 0)
	{
		tty_write(buffer, count, (size_t)files[handle].path[8] - 48);
		release_lock(&vfs_mutex);
		return count;
	} else if(strcmp(files[handle].path, "/dev/port") == 0)
	{
		// write to I/O ports here!
		if(count == 1)
		{
			byte = (uint8_t*)buffer;
			outb((uint16_t)files[handle].position, byte[0]);
		} else if(count == 2)
		{
			word = (uint16_t*)buffer;
			outw((uint16_t)files[handle].position, word[0]);
		} else if(count == 4)
		{
			dword = (uint32_t*)buffer;
			outd((uint16_t)files[handle].position, dword[0]);
		} else
		{
			kprintf("devfs: attempted to write undefined size %d to I/O port 0x%xw\n", count, (uint16_t)files[handle].position);
			release_lock(&vfs_mutex);
			return EIO;
		}

		release_lock(&vfs_mutex);
		return count;
	}

	release_lock(&vfs_mutex);
	return 0;
}





