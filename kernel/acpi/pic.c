
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <kprintf.h>
#include <io.h>
#include <pic.h>
#include <apic.h>

// pic_init(): Initializes the PICs
// Param:	uint8_t base - base of interrupts
// Return:	Nothing

void pic_init(uint8_t base)
{
	if(base == IRQ_BASE)
		kprintf("pic: using the dual PIC as an interrupt controller.\n");

	// mask the PICs
	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);
	iowait();

	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	iowait();

	outb(0x21, base);
	outb(0xA1, base+8);
	iowait();

	outb(0x21, 4);
	outb(0xA1, 2);
	iowait();

	outb(0x21, 1);
	outb(0xA1, 1);
	iowait();

	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);
	iowait();

	// EOI any existing IRQs
	outb(0x20, 0x20);
	iowait();
	outb(0x20, 0x20);
	iowait();
	outb(0x20, 0x20);
	iowait();
	outb(0xA0, 0x20);
	iowait();
	outb(0xA0, 0x20);
	iowait();
	outb(0xA0, 0x20);
	iowait();

	// unmask the cascade to allow IRQs from the slave PIC
	pic_unmask(2);
}

// pic_mask(): Masks an IRQ
// Param:	uint8_t irq - IRQ line
// Return:	Nothing

void pic_mask(uint8_t irq)
{
	uint8_t data;

	if(irq >= 8)
	{
		// slave PIC
		irq -= 8;
		data = inb(0xA1);
		data |= (1 << irq);
		outb(0xA1, data);
		iowait();
	} else
	{
		// master PIC
		data = inb(0x21);
		data |= (1 << irq);
		outb(0x21, data);
		iowait();
	}
}

// pic_unmask(): Unmasks an IRQ
// Param:	uint8_t irq - IRQ line
// Return:	Nothing

void pic_unmask(uint8_t irq)
{
	uint8_t data;

	if(irq >= 8)
	{
		// slave PIC
		irq -= 8;
		data = inb(0xA1);
		data &= ~(1 << irq);
		outb(0xA1, data);
		iowait();
	} else
	{
		// master PIC
		data = inb(0x21);
		data &= ~(1 << irq);
		outb(0x21, data);
		iowait();
	}
}

// pic_eoi(): Sends end of interrupt command
// Param:	uint8_t irq - IRQ line
// Return:	Nothing

void pic_eoi(uint8_t irq)
{
	if(irq >= 8)
		outb(0xA0, 0x20);

	outb(0x20, 0x20);
}




