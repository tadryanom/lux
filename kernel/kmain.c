
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
	devmgr_dump();

	while(1)
		asm volatile ("sti\nhlt");
}



