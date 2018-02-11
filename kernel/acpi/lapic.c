
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <mm.h>
#include <kprintf.h>

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




