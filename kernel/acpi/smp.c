
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <mm.h>
#include <kprintf.h>
#include <string.h>	// memcpy
#include <cpu.h>

int smp_boot_ap(size_t);
void smp_wait();

size_t current_ap;		// this will tell the APs which index they are
uint8_t ap_flag;		// this will tell the BSP if the AP started up

cpu_t *cpus;

// smp_init(): Initializes application processors
// Param:	Nothing
// Return:	Nothing

void smp_init()
{
	cpus = kcalloc(sizeof(cpu_t), MAX_LAPICS);
	cpus[0].index = 0;
	cpus[0].stack = kmalloc(STACK_SIZE);

	kprintf("smp: total of %d usable CPUs present.\n", lapic_count);
	if(lapic_count <= 1)
	{
		kprintf("smp: no application processors present, nothing to do.\n");
		return;
	}

	// copy the trampoline code into low memory
	memcpy((void*)0x1000, trampoline16, trampoline16_size[0]);

	size_t i = 0;
	for(i = 0; i < lapic_count; i++)
		smp_boot_ap(i);
}

// smp_wait(): Waits for IPIs to be sent
// Param:	Nothing
// Return:	Nothing

void smp_wait()
{
	int i = 0;
	while(i < 0xFFFF)
	{
		write_cr0(read_cr0());
		i++;
	}
}

// smp_boot_ap(): Starts an application processor
// Param:	size_t ap - processor index
// Return:	int - 0 on success

int smp_boot_ap(size_t ap)
{
	if(lapics[ap].apic_id == 0)	// BSP?
		return 0;		// skip -- the BSP is obviously running already

	current_ap = ap;
	uint8_t apic_id = lapics[ap].apic_id;
	//kprintf("smp: attempting to start up CPU index %d APIC ID 0x%xb\n", ap, apic_id);

	ap_flag = 0;

	// send the AP an INIT IPI
	lapic_write(LAPIC_COMMAND_ID, (uint32_t)(apic_id & 0xF) << 24);
	lapic_write(LAPIC_COMMAND, 0x4500);
	smp_wait();

	// send a startup IPI
	lapic_write(LAPIC_COMMAND_ID, (uint32_t)(apic_id & 0xF) << 24);
	lapic_write(LAPIC_COMMAND, 0x4601);
	smp_wait();

	size_t i = 0;
	while(i < 0xFF)
	{
		smp_wait();
		if(ap_flag == 1)
			return 0;
		else
			i++;

		continue;
	}

	kprintf("smp: CPU index %d didn't respond to SIPI.\n", ap);
	return 1;
}

// smp_kmain(): Kernel entry point for application processors
// Param:	Nothing
// Return:	Nothing

void smp_kmain()
{
	// for 32-bit, we need to enable paging because the trampoline code
	// doesn't do it. But for 64-bit, it has to because paging is always
	// enabled in long mode.

#if __i386__
	write_cr3((uint32_t)page_directory);
	uint32_t cr0 = read_cr0();
	cr0 |= 0x80000000;
	cr0 &= ~0x60000000;		// caching
	write_cr0(cr0);
#endif

	kprintf("smp: CPU index %d started up.\n", current_ap);

	ap_flag = 1;
	while(1);
}



