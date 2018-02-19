
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
#include <gdt.h>
#include <idt.h>

int smp_boot_ap(size_t);
void smp_wait();

size_t current_ap;		// this will tell the APs which index they are
uint8_t ap_flag;		// this will tell the BSP if the AP started up

// smp_init(): Initializes application processors
// Param:	Nothing
// Return:	Nothing

void smp_init()
{
	kprintf("smp: total of %d usable CPUs present.\n", lapic_count);

	idt_install(0xFF, (size_t)&lapic_spurious_stub);

	// register the bsp
	smp_register_cpu(0);

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

	// all AP local APICs are initialized to logical APIC mode & spurious IRQs
	// we couldn't do the same for the BSP because we're using it to start
	// APs, so we need physical mode - but now we can properly initialize the
	// BSP's local APIC
	lapic_init();
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
	kprintf("smp: CPU index %d started up.\n", current_ap);
	smp_register_cpu(current_ap);

	lapic_init();

	ap_flag = 1;

	while(1)
		asm volatile ("sti\nhlt");
}

// smp_register_cpu(): Registers a CPU that has started up
// Modifies the GDT in 32-bit mode, uses MSR_FS_BASE in 64-bit mode
// Param:	size_t index - CPU index
// Return:	Nothing

void smp_register_cpu(size_t index)
{
	cpu_t *cpu = kmalloc(sizeof(cpu_t));
	cpu->index = index;
	cpu->stack = kmalloc(STACK_SIZE) + STACK_SIZE;

#if __i386__
	gdt_set_entry(GDT_CPU_INFO + index, (uint32_t)cpu, GDT_ACCESS_PRESENT | GDT_ACCESS_RW, GDT_FLAGS_PMODE);
	gdt_set_limit(GDT_CPU_INFO + index, sizeof(cpu_t));
	flush_gdt(gdtr, 0x08, 0x10);
	load_fs((GDT_CPU_INFO + index) << 3);
#endif

#if __x86_64__
	write_msr(MSR_FS_BASE, (uint64_t)cpu);
#endif
}



