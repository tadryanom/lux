
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <time.h>

#define MAX_FILES			512
#define MAX_MOUNTPOINTS			16

// kernel-supported filesystems someday
#define FS_EXT2				1
#define FS_EXT3				2
#define FS_EXT4				3
#define FS_FAT16			4
#define FS_FAT32			5
#define FS_ISO9660			6

// open() flags
#define O_RDONLY			0x0001
#define O_WRONLY			0x0002
#define O_RDWR				(O_RDONLY | O_WRONLY)
#define O_ACCMODE			(~O_RDWR)
#define O_APPEND			0x0004
#define O_NONBLOCK			0x0000		// we don't use this
#define O_NDELAY			O_NONBLOCK
#define O_FSYNC				0x0000		// not using this either
#define O_SYNC				O_FSYNC
#define O_NOATIME			0x0008
#define O_CREAT				0x0010
#define O_EXCL				0x0020
#define O_TMPFILE			0x0040		// we'll fail when we use this
#define O_NOCTTY			0x0000		// not using this
#define O_EXLOCK			0x0080
#define O_SHLOCK			0x0100

// Values of mode_t
#define S_IFBLK				0x0001		// block special
#define S_IFCHR				0x0002		// char special
#define S_IFIFO				0x0004		// fifo special
#define S_IFREG				0x0008		// regular file
#define S_IFDIR				0x0010		// directory
#define S_IFLNK				0x0020		// symbolic link

#define S_IRUSR				0x0040		// read, owner
#define S_IWUSR				0x0080		// write, owner
#define S_IXUSR				0x0100		// execute, owner
#define S_IRWXU				(S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IRGRP				0x0200		// read, group
#define S_IWGRP				0x0400		// write, group
#define S_IXGRP				0x0800		// execute, group
#define S_IRWXG				(S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IROTH				0x1000		// read, others
#define S_IWOTH				0x2000		// write, others
#define S_IXOTH				0x4000		// execute, others
#define S_IRWXO				(S_IROTH | S_IWOTH | S_IXOTH)

#define STDIN				0
#define STDOUT				1
#define STDERR				2

typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t gid_t;
typedef uint32_t uid_t;
typedef uint32_t dev_t;
typedef uint32_t nlink_t;
typedef size_t off_t;
typedef uint16_t blksize_t;
typedef size_t blkcnt_t;

typedef struct file_handle_t
{
	char present;
	char path[1024];
	size_t position;
	int flags;
	pid_t pid;
} file_handle_t;

file_handle_t *files;

typedef struct directory_t
{
	char path[1024];
	size_t index;
	size_t size;
} directory_t;

typedef struct directory_entry_t
{
	char filename[1024];
} directory_entry_t;

typedef struct mountpoint_t
{
	char present;
	uint16_t fs_type;
	char path[1024];
	char device[64];	// '/dev/hdxx'
	uid_t uid;
	gid_t gid;
} mountpoint_t;

struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	//dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
};

file_handle_t *files;
mountpoint_t *mountpoints;
char full_path[1024];

void vfs_init();
void vfs_make_full_path();

// Public functions
int open(const char *, int, ...);
int read(int, char *, int);
int write(int, char *, int);
int link(char *, char *);
int unlink(char *);
int lseek(int, int, int);
int chmod(const char *, mode_t);
int fchmod(int, mode_t);
int stat(const char *, struct stat *);
int mkdir(const char *, mode_t);

// Non-standard functions
directory_t *dir_open(char *);
void dir_close(directory_t *);
int dir_query(directory_t *, directory_entry_t *);



