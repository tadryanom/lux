
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <kprintf.h>
#include <pic.h>

void ioapic_init_controller(size_t, uint8_t);
void ioapic_write_irq(size_t, uint8_t, uint64_t);
uint64_t ioapic_read_irq(size_t, uint8_t);

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

	volatile uint32_t *selector = (uint32_t*)ptr;
	volatile uint32_t *buffer = (uint32_t*)(ptr + 16);

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

	volatile uint32_t *selector = (uint32_t*)ptr;
	volatile uint32_t *buffer = (uint32_t*)(ptr + 16);

	selector[0] = index;
	buffer[0] = value;
}

// ioapic_init(): Initializes the I/O APICs
// Param:	Nothing
// Return:	Nothing

void ioapic_init()
{
	kprintf("ioapic: using the I/O APIC as an interrupt controller.\n");

	// disable the PIC by mapping it to INT 0x20-0x2F
	pic_init(UNUSED_PIC_BASE);

	// now initialize all present I/O APICs
	size_t i;
	uint8_t count = 0;
	for(i = 0; i < ioapic_count; i++)
	{
		ioapic_init_controller(i, IRQ_BASE + count);
		count += ioapics[i].irq_count;
	}
}

// ioapic_init_controller(): Initializes a single I/O APIC
// Param:	size_t index - I/O APIC index
// Param:	uint8_t base - IRQ base
// Return:	Nothing

void ioapic_init_controller(size_t index, uint8_t base)
{
	kprintf("ioapic[%d]: initialize I/O APIC with interrupt range 0x%xb-0x%xb\n", index, base, base + ioapics[index].irq_count);
	size_t i = 0;
	while(i < ioapics[index].irq_count)
	{
		ioapic_write_irq(index, i, (uint64_t)(base + i) | IOAPIC_MASK);
		i++;
	}
}

// ioapic_write_irq(): Writes to an IRQ pair of registers
// Param:	size_t index - I/O APIC index
// Param:	uint8_t irq - IRQ line
// Param:	uint64_t value - value to write
// Return:	Nothing

void ioapic_write_irq(size_t index, uint8_t irq, uint64_t value)
{
	uint32_t reg = (irq << 1) + IOAPIC_IRQ_TABLE;		// two 32-bit registers for each IRQ
	ioapic_write(index, reg, (uint32_t)value & 0xFFFFFFFF);
	ioapic_write(index, reg+1, (uint32_t)(value >> 32) & 0xFFFFFFFF);
}

// ioapic_read_irq(): Reads an IRQ pair of registers
// Param:	size_t index - I/O APIC index
// Param:	uint8_t irq - IRQ line
// Return:	uint64_t value - value from registers

uint64_t ioapic_read_irq(size_t index, uint8_t irq)
{
	uint32_t low, high;
	uint32_t reg = (irq << 1) + IOAPIC_IRQ_TABLE;
	low = ioapic_read(index, reg);
	high = ioapic_read(index, reg+1);

	uint64_t value = (uint64_t)((uint64_t)high << 32) | low;
	return value;
}

// ioapic_mask(): Masks a GSI
// Param:	uint8_t gsi - GSI IRQ line
// Return:	Nothing

void ioapic_mask(uint8_t gsi)
{
	// first figure out which I/O APIC has this GSI
	size_t ioapic = 0;
	while(gsi < ioapics[ioapic].gsi || gsi > (ioapics[ioapic].gsi + ioapics[ioapic].irq_count))
	{
		ioapic++;
		if(ioapics[ioapic].present != 1)
			return;
	}

	// determine the interrupt number of this specific IRQ
	uint8_t irq = (uint8_t)gsi - ioapics[ioapic].gsi;
	kprintf("ioapic[%d]: mask IRQ line %d\n", ioapic, (int)irq);

	uint64_t value = ioapic_read_irq(ioapic, irq);
	value |= IOAPIC_MASK;
	ioapic_write(ioapic, irq, value);
}

// ioapic_unmask(): Unmasks a GSI
// Param:	uint8_t gsi - GSI IRQ line
// Return:	Nothing

void ioapic_unmask(uint8_t gsi)
{
	// first figure out which I/O APIC has this GSI
	size_t ioapic = 0;
	while(gsi < ioapics[ioapic].gsi || gsi > (ioapics[ioapic].gsi + ioapics[ioapic].irq_count))
	{
		ioapic++;
		if(ioapics[ioapic].present != 1)
			return;
	}

	// determine the interrupt number of this specific IRQ
	uint8_t irq = (uint8_t)gsi - ioapics[ioapic].gsi;
	kprintf("ioapic[%d]: unmask IRQ line %d\n", ioapic, (int)irq);

	uint64_t value = ioapic_read_irq(ioapic, irq);
	value &= ~IOAPIC_MASK;
	ioapic_write_irq(ioapic, irq, value);
}




