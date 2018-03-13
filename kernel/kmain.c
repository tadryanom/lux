
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <boot.h>
#include <kprintf.h>
#include <devmgr.h>
#include <timer.h>
#include <mm.h>
#include <gdt.h>
#include <tty.h>
#include <acpi.h>
#include <apic.h>
#include <vfs.h>
#include <tasking.h>
#include <blkdev.h>
#include <string.h>
#include <rand.h>

void *kend;

void kmain(uint32_t multiboot_magic, multiboot_info_t *multiboot_info, vbe_mode_t *vbe_mode)
{
	kprint_init();

	if(multiboot_magic != MULTIBOOT_MAGIC)
		kprintf("warning: invalid multiboot magic; taking a risk and continuing...\n");

	// set *kend to the start of free memory, after GRUB modules
	uint32_t mod_count = 0;
	multiboot_module_t *module;
	if(((multiboot_info->flags & MULTIBOOT_FLAGS_MODULES) != 0) && multiboot_info->mods_count != 0)
	{
		module = (multiboot_module_t*)((size_t)multiboot_info->mods_addr);

		while(mod_count < multiboot_info->mods_count)
		{
			kend = (void*)((size_t)module[mod_count].mod_end + 0x100000);
			mod_count++;
		}
	} else
	{
#if __i386__
	kend = (void*)0x400000;
#endif

#if __x86_64__
	kend = (void*)0x400000 + PHYSICAL_MEMORY;
#endif
	}

	mm_init(multiboot_info);
	devmgr_init();
	screen_init(vbe_mode);
	gdt_init();
	install_exceptions();
	acpi_init();
	apic_init();
	timer_init();
	tasking_init();
	vfs_init();
	blkdev_init(multiboot_info);
	mount("/dev/initrd", "/", "ustar", 0, 0);
	//devmgr_dump();

	/*int file = open("/hello.txt", O_RDONLY);
	kprintf("opened file hello.txt, file handle %d\n", file);*/

	struct stat stat_info;
	int status = stat("/hello.txt", &stat_info);
	kprintf("stat '/hello.txt' returned status code: %d\n", status);

	kprintf("  file mode: 0x%xd (", stat_info.st_mode);
	if(stat_info.st_mode & S_IFBLK)
		kprintf("b");
	else if(stat_info.st_mode & S_IFCHR)
		kprintf("c");
	else if(stat_info.st_mode & S_IFDIR)
		kprintf("d");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IRUSR)
		kprintf("r");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IWUSR)
		kprintf("w");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IXUSR)
		kprintf("x");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IRGRP)
		kprintf("r");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IWGRP)
		kprintf("w");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IXGRP)
		kprintf("x");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IROTH)
		kprintf("r");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IWOTH)
		kprintf("w");
	else
		kprintf("-");

	if(stat_info.st_mode & S_IXOTH)
		kprintf("x");
	else
		kprintf("-");

	kprintf(")\n");
	kprintf("  file size: %d\n", stat_info.st_size);

	kprintf("Boot finished, %d MB used, %d MB free\n", used_pages/256, (total_pages-used_pages) / 256);

	while(1)
		asm volatile ("sti\nhlt");
}



