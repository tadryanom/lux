
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <vfs.h>
#include <mm.h>
#include <kprintf.h>
#include <string.h>
#include <lock.h>

// vfs_determine_mountpoint(): Determines the mountpoint of a path
// Param:	char *path - fully resolved path
// Return:	int - mountpoint index containing requested path, -1 on error

int vfs_determine_mountpoint(char *path)
{
	int mountpoint = 0, mountpoint2 = 0;
	size_t size = 0, size2 = 0;

	while(mountpoint < MAX_MOUNTPOINTS)
	{
		if(mountpoints[mountpoint].present != 1)
		{
			mountpoint++;
			continue;
		}

		size = strlen(mountpoints[mountpoint].path);
		if(memcmp(mountpoints[mountpoint].path, path, size) == 0)
		{
			// keep the longest path
			if(size > size2)
			{
				size2 = size;
				mountpoint2 = mountpoint;
			}
		}

		mountpoint++;
	}

	if(size2 != 0 && mountpoint2 < MAX_MOUNTPOINTS)
		return mountpoint2;

	else
		return -1;
}

// mount(): Mounts a filesystem
// Param:	const char *device - special block device file
// Param:	const char *dir - directory to mount on
// Param:	const char *fstype - type of filesystem
// Param:	unsigned long int flags - mount flags
// Param:	void *data - unused
// Return:	int - status code

int mount(const char *device, const char *dir, const char *fstype, unsigned long int flags, void *data)
{
	if(!flags & MS_MGC_MASK)
	{
		flags = 0;
		data = (void*)0;
	}

	// ensure existence of device and dir
	struct stat stat_info;
	int status = stat(device, &stat_info);
	if(status != 0)
		return status;

	if(!stat_info.st_mode & S_IFBLK)
		return ENOTBLK;

	status = stat(dir, &stat_info);
	if(status != 0)
		return status;

	if(!stat_info.st_mode & S_IFDIR)
		return ENOTDIR;

	acquire_lock(&vfs_mutex);

	// find an empty mountpoint
	int mountpoint = 0;
	while(mountpoints[mountpoint].present != 0 && mountpoint < MAX_MOUNTPOINTS)
		mountpoint++;

	if(mountpoint >= MAX_MOUNTPOINTS)
	{
		release_lock(&vfs_mutex);
		return ENOBUFS;
	}

	// create the mountpoint structure
	mountpoints[mountpoint].present = 1;
	strcpy(mountpoints[mountpoint].fstype, fstype);

	vfs_resolve_path(full_path, device);
	strcpy(mountpoints[mountpoint].device, full_path);

	vfs_resolve_path(full_path, dir);
	strcpy(mountpoints[mountpoint].path, full_path);

	mountpoints[mountpoint].flags = flags;

	// TO-DO: UID and GID stuff here!

	kprintf("vfs: mounted %s on %s, filesystem type '%s'\n", device, dir, fstype);
	release_lock(&vfs_mutex);
	return 0;
}




