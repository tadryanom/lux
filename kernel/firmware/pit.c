
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <timer.h>
#include <kprintf.h>
#include <irq.h>
#include <devmgr.h>
#include <mm.h>
#include <io.h>

#define PIT_DIVIDER			1193182

// pit_init(): Initializes the PIT
// Param:	Nothing
// Return:	Nothing

void pit_init()
{
	kprintf("pit: using the PIT as a timer.\n");

	// set frequency and mode
	uint16_t divider = PIT_DIVIDER / TIMER_FREQUENCY;

	outb(0x43, 0x36);
	iowait();
	outb(0x40, (uint8_t)divider & 0xFF);
	iowait();
	outb(0x40, (uint8_t)(divider >> 8) & 0xFF);
	iowait();

	// broadcast the IRQ to all CPUs
	timer_irq_line = irq_configure(0, IRQ_BROADCAST);

	irq_install(timer_irq_line, (size_t)&timer_irq_stub);
	irq_unmask(timer_irq_line);

	// register the device
	device_t *device = kmalloc(sizeof(device_t));
	device->category = DEVMGR_CATEGORY_SYSTEM;
	device->irq = timer_irq_line;

	device->io[0].base = 0x40;
	device->io[0].size = 4;

	devmgr_register(device, "8253/8254 timer");
	kfree(device);
}



