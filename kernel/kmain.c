
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
#include <time.h>

void kmain(uint32_t multiboot_magic, multiboot_info_t *multiboot_info, vbe_mode_t *vbe_mode)
{
	kprint_init();

	if(multiboot_magic != MULTIBOOT_MAGIC)
		kprintf("warning: invalid multiboot magic; taking a risk and continuing...\n");

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
	//ps2_init();
	//devmgr_dump();

	struct stat statstruc;
	stat("/dev/stdin", &statstruc);

	kprintf("stat information for /dev/stdin:\n");
	kprintf("st_mode: %xd (", statstruc.st_mode);

	if(statstruc.st_mode & S_IFDIR)
		kprintf("d");
	else if(statstruc.st_mode & S_IFCHR)
		kprintf("c");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IRUSR)
		kprintf("r");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IWUSR)
		kprintf("w");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IXUSR)
		kprintf("x");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IRGRP)
		kprintf("r");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IWGRP)
		kprintf("w");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IXGRP)
		kprintf("x");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IROTH)
		kprintf("r");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IWOTH)
		kprintf("w");
	else
		kprintf("-");

	if(statstruc.st_mode & S_IXOTH)
		kprintf("x");
	else
		kprintf("-");

	kprintf(")\n");
	kprintf("st_atime: %d\n", statstruc.st_atime);


	while(1)
		asm volatile ("sti\nhlt");
}



