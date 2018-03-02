
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <ustar.h>
#include <vfs.h>
#include <kprintf.h>
#include <string.h>
#include <mm.h>

// The kernel calls filesystem driver providing it a fully-resolved path or a
// pointer to a file handle structure, and a pointer to a mountpoint structure
// in kernel memory. The filesystem driver uses this information to read/write
// raw bytes on the actual disk, using /dev/hdxpx or /dev/initrd.

// ustar_get_file(): Internal function, returns pointer in bytes to a USTAR file entry
// Param:	mountpoint_t *mountpoint - pointer to mountpoint structure
// Param:	const char *path - path of file/directory
// Param:	ustar_entry_t *destination - destination to copy entry info
// Return:	uint64_t - offset in bytes, 1 on error

uint64_t ustar_get_file(mountpoint_t *mountpoint, const char *path, ustar_entry_t *destination)
{
	uint64_t block = 0;
	size_t file_size;

	// open the disk device for reading
	int handle = open(mountpoint->device, O_RDONLY);
	ustar_entry_t *entry = kmalloc(sizeof(ustar_entry_t));

	while(read(handle, (char*)entry, sizeof(ustar_entry_t)) == sizeof(ustar_entry_t))
	{
		if(memcmp(entry->signature, "ustar", 5) != 0)
			break;

		if(strcmp(entry->name, path) == 0)
		{
			close(handle);
			memcpy(destination, entry, sizeof(ustar_entry_t));
			kfree(entry);
			return block * USTAR_BLOCK_SIZE;
		}

		file_size = oct_to_dec(entry->size);
		block += (file_size + USTAR_BLOCK_SIZE - 1) / USTAR_BLOCK_SIZE;
		block++;

		lseek(handle, block * USTAR_BLOCK_SIZE, SEEK_SET);
	}

	close(handle);
	kfree(entry);
	return 1;
}

// ustar_stat(): stat() function for USTAR filesystem
// Param:	mountpoint_t *mountpoint - pointer to mountpoint structure
// Param:	const char *path - path of file/directory
// Param:	struct stat *destination - destination to store stat information
// Return:	int - status code

int ustar_stat(mountpoint_t *mountpoint, const char *path, struct stat *destination)
{
	// skip to the actual path
	path += strlen(mountpoint->path);

	// get the file entry
	ustar_entry_t entry;
	uint64_t offset = ustar_get_file(mountpoint, path, &entry);
	if(offset == 1)
		return ENOENT;

	if(strcmp(mountpoint->device, "/dev/initrd") == 0)
		destination->st_dev = 0;
	else
		destination->st_dev = (dev_t)mountpoint->device[7] - 48;

	path -= strlen(mountpoint->path);

	destination->st_ino = offset / USTAR_BLOCK_SIZE;	// not really inodes, but okay
	destination->st_nlink = 0;		// TO-DO...
	destination->st_uid = oct_to_dec(entry.uid);
	destination->st_gid = oct_to_dec(entry.gid);
	destination->st_size = oct_to_dec(entry.size);
	destination->st_mtime = oct_to_dec(entry.mtime);
	destination->st_ctime = oct_to_dec(entry.mtime);
	destination->st_atime = get_time();
	destination->st_blksize = USTAR_BLOCK_SIZE;
	destination->st_blocks = (destination->st_size + USTAR_BLOCK_SIZE - 1) / USTAR_BLOCK_SIZE;

	destination->st_mode = 0;

	switch(entry.type)
	{
	case USTAR_REG:
	case 0:
		destination->st_mode |= S_IFREG;
		break;
	case USTAR_HARD_LINK:
	case USTAR_SYMLINK:
		destination->st_mode |= S_IFLNK;
		break;
	case USTAR_CHR:
		destination->st_mode |= S_IFCHR;
		break;
	case USTAR_BLK:
		destination->st_mode |= S_IFBLK;
		break;
	case USTAR_DIR:
		destination->st_mode |= S_IFDIR;
		break;
	case USTAR_FIFO:
		destination->st_mode |= S_IFIFO;
		break;
	default:
		kprintf("ustar: %s: unknown file type %xb, ignoring...\n", path, entry.type);
		break;
	}

	// now the file permissions
	size_t permissions = oct_to_dec(entry.mode);
	if(permissions & USTAR_READ_USER)
		destination->st_mode |= S_IRUSR;

	if(permissions & USTAR_WRITE_USER)
		destination->st_mode |= S_IWUSR;

	if(permissions & USTAR_EXECUTE_USER)
		destination->st_mode |= S_IXUSR;

	if(permissions & USTAR_READ_GROUP)
		destination->st_mode |= S_IRGRP;

	if(permissions & USTAR_WRITE_GROUP)
		destination->st_mode |= S_IWGRP;

	if(permissions & USTAR_EXECUTE_GROUP)
		destination->st_mode |= S_IXGRP;

	if(permissions & USTAR_READ_OTHER)
		destination->st_mode |= S_IROTH;

	if(permissions & USTAR_WRITE_OTHER)
		destination->st_mode |= S_IWOTH;

	if(permissions & USTAR_EXECUTE_OTHER)
		destination->st_mode |= S_IXOTH;

	return 0;
}






