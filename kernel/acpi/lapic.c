
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <mm.h>
#include <kprintf.h>
#include <cpu.h>

void *lapic_base;

// lapic_read(): Reads a local APIC register
// Param:	size_t index - index of register
// Return:	uint32_t - value from register

uint32_t lapic_read(size_t index)
{
	volatile uint32_t *ptr = (uint32_t*)(lapic_base + index);
	return ptr[0];
}

// lapic_write(): Writes to a local APIC register
// Param:	size_t index - index of register
// Param:	uint32_t value - value to write
// Return:	Nothing

void lapic_write(size_t index, uint32_t value)
{
	volatile uint32_t *ptr = (uint32_t*)(lapic_base + index);
	ptr[0] = value;
}

// lapic_get_id(): Returns APIC ID of current CPU
// Param:	Nothing
// Return:	uint8_t - local APIC ID

uint8_t lapic_get_id()
{
	cpu_t FS_BASE *cpu = (cpu_t FS_BASE *)0;
	return lapics[cpu->index].apic_id;
}

// lapic_init(): Initializes the local APIC
// Param:	Nothing
// Return:	Nothing

void lapic_init()
{
	lapic_write(LAPIC_TASK_PRIORITY, 0);
	lapic_write(LAPIC_SPURIOUS_IRQ, 0x1FF);		// enable spurious IRQ at INT 0xFF
	lapic_write(LAPIC_DFR, 0xFFFFFFFF);		// destination format = flat
	lapic_write(LAPIC_LDR, (uint32_t)LAPIC_CLUSTER_ID << 24);

	// send some EOIs, just in case...
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
	lapic_write(LAPIC_EOI, 0);
}

// lapic_eoi(): Sends an EOI
// Param:	Nothing
// Return:	Nothing

inline void lapic_eoi()
{
	lapic_write(LAPIC_EOI, 0);
}

// lapic_spurious(): Local APIC spurious IRQ handler
// Param:	Nothing
// Return:	Nothing

void lapic_spurious()
{
	cpu_t FS_BASE *cpu = (cpu_t FS_BASE*)0;
	cpu->spurious++;
	kprintf("lapic: spurious IRQ on CPU index %d, total count %d\n", cpu->index, cpu->spurious);
}





