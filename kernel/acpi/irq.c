
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <pic.h>
#include <apic.h>
#include <kprintf.h>
#include <idt.h>

// irq_eoi(): Sends an EOI
// Param:	uint8_t irq - IRQ number
// Return:	Nothing

void irq_eoi(uint8_t irq)
{
	if(irq_mode == IRQ_IOAPIC)
		return lapic_eoi();

	else if(irq_mode == IRQ_PIC)
		return pic_eoi(irq);
}

// irq_mask(): Masks an IRQ
// Param:	uint8_t irq - IRQ number
// Return:	Nothing

void irq_mask(uint8_t irq)
{
	if(irq_mode == IRQ_IOAPIC)
		return ioapic_mask(irq);

	else if(irq_mode == IRQ_PIC)
		return pic_mask(irq);
}

// irq_unmask(): Unmasks an IRQ
// Param:	uint8_t irq - IRQ number
// Return:	Nothing

void irq_unmask(uint8_t irq)
{
	if(irq_mode == IRQ_IOAPIC)
		return ioapic_unmask(irq);

	else if(irq_mode == IRQ_PIC)
		return pic_unmask(irq);
}

// irq_configure(): Configures an IRQ
// Param:	uint8_t irq - IRQ number
// Param:	uint8_t flags - IRQ configuration
// Return:	uint8_t - GSI of the IRQ

uint8_t irq_configure(uint8_t irq, uint8_t flags)
{
	if(irq_mode == IRQ_IOAPIC)
		return ioapic_configure(irq, flags);

	// for the old PIC, there's nothing to do here
	return irq;
}

// irq_install(): Installs an IRQ handler
// Param:	uint8_t irq - IRQ number
// Param:	size_t handler - address of handler
// Return:	Nothing

void irq_install(uint8_t irq, size_t handler)
{
	idt_install(irq + IRQ_BASE, handler);
}





