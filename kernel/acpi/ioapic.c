
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <mm.h>
#include <kprintf.h>

// ioapic_read(): Reads an I/O APIC register
// Param:	size_t ioapic - I/O APIC index
// Param:	uint32_t index - index of register
// Return:	uint32_t - value from register

uint32_t ioapic_read(size_t ioapic, uint32_t index)
{
	if(ioapics[ioapic].present != 1)
	{
		kprintf("ioapic[%d]: attempted to read from non-existant I/O APIC, returning zero.\n", ioapic);
		return 0;
	}

	void *ptr = (void*)ioapics[ioapic].base;

	uint32_t *selector = (uint32_t*)ptr;
	uint32_t *buffer = (uint32_t*)(ptr + 16);

	selector[0] = index;
	return buffer[0];
}

// ioapic_write(): Writes an I/O APIC register
// Param:	size_t ioapic - I/O APIC index
// Param:	uint32_t index - index of register
// Param:	uint32_t value - value to write
// Return:	Nothing

void ioapic_write(size_t ioapic, uint32_t index, uint32_t value)
{
	if(ioapics[ioapic].present != 1)
	{
		kprintf("ioapic[%d]: attempted to write %xd to non-existant I/O APIC.\n", ioapic, value);
		return;
	}

	void *ptr = (void*)ioapics[ioapic].base;

	uint32_t *selector = (uint32_t*)ptr;
	uint32_t *buffer = (uint32_t*)(ptr + 16);

	selector[0] = index;
	buffer[0] = value;
}





