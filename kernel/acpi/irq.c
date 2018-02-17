
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <pic.h>
#include <apic.h>
#include <kprintf.h>

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
		return pic_mask(irq);
}




