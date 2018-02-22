
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <ps2.h>
#include <devmgr.h>
#include <tty.h>
#include <kprintf.h>
#include <mm.h>
#include <io.h>

void ps2_kbd_init();

// ps2_wait_read(): Waits to read from the PS/2 controller
// Param:	Nothing
// Return:	int - 0 on success, 1 on timeout

int ps2_wait_read()
{
	size_t waits;

	for(waits = 0; waits < 0xFFFFF; waits++)
	{
		asm volatile ("pause");
		if((inb(0x64) & 1) != 0)
			return 0;
	}

	kprintf("ps2: read timeout.\n");
	return 1;
}

// ps2_wait_write(): Waits to write to the PS/2 controller
// Param:	Nothing
// Return:	int - 0 on success, 1 on timeout

int ps2_wait_write()
{
	size_t waits;

	for(waits = 0; waits < 0xFFFFF; waits++)
	{
		asm volatile ("pause");
		if((inb(0x64) & 2) == 0)
			return 0;
	}

	kprintf("ps2: write timeout.\n");
	return 1;
}

// ps2_init(): Detects and initializes PS/2 devices
// Param:	Nothing
// Return:	Nothing

void ps2_init()
{
	if(inb(0x64) == 0xFF)
		return;		// PS/2 controller not present

	device_t *device = kmalloc(sizeof(device_t));
	device->category = DEVMGR_CATEGORY_SYSTEM;
	device->io[0].base = 0x60;
	device->io[0].size = 1;
	device->io[1].base = 0x64;
	device->io[1].size = 1;

	devmgr_register(device, "Standard PS/2 controller");

	ps2_kbd_init();
	//ps2_mouse_init();
}

// ps2_kbd_init(): Initializes the PS/2 keyboard
// Param:	Nothing
// Return:	Nothing

void ps2_kbd_init()
{
	kprintf("ps2: initialize PS/2 keyboard.\n");

	// TO-DO: Finish this.
}







