
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
#include <ps2.h>
#include <vfs.h>
#include <tasking.h>
#include <blkdev.h>
#include <string.h>

void *kend;

void kmain(uint32_t multiboot_magic, multiboot_info_t *multiboot_info, vbe_mode_t *vbe_mode)
{
	kprint_init();

	if(multiboot_magic != MULTIBOOT_MAGIC)
		kprintf("warning: invalid multiboot magic; taking a risk and continuing...\n");

	// set *kend to the start of free memory, after GRUB modules
	uint32_t mod_count = 0;
	multiboot_module_t *module;
	if(multiboot_info->flags & MULTIBOOT_FLAGS_MODULES && multiboot_info->mods_count != 0)
	{
		module = (multiboot_module_t*)((size_t)multiboot_info->mods_addr);

		while(mod_count < multiboot_info->mods_count)
		{
			kend = (void*)((size_t)module[mod_count].mod_end);
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
	//ps2_init();
	//devmgr_dump();

	int file = open("/dev/stdout", O_RDWR);
	kprintf("opened '/dev/stdout', file handle %d\n", file);

	char text[] = "\nHello, world.\nThis is being written using write() and not kprintf()\n\nNew line above.";

	write(file, text, strlen(text));

	while(1)
		asm volatile ("sti\nhlt");
}



