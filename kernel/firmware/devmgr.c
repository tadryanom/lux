
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <kprintf.h>
#include <devmgr.h>
#include <mm.h>
#include <string.h>

device_t *devices;
extern uint16_t com1_base;
size_t device_count = 0;

char *device_types[] = {
	"undefined",			// 0
	"system",			// 1
	"CPU",				// 2
	"graphics",			// 3
	"network",			// 4
	"disk controller",		// 5
	"disk",				// 6
	"keyboard",			// 7
	"mouse",			// 8
	"USB controller",		// 9
	"USB",				// 10
	"communications",		// 11
};

// devmgr_init(): Initializes the device manager
// Param:	Nothing
// Return:	Nothing

void devmgr_init()
{
	devices = kcalloc(sizeof(device_t), MAX_DEVICES);
}

// devmgr_register(): Registers a device
// Param:	device_t *device - device to register
// Param:	char *name - name of device
// Return:	Nothing

void devmgr_register(device_t *device, char *name)
{
	if(strlen(name) > 127 || device_count >= MAX_DEVICES || device->category > DEVMGR_LAST_CATEGORY || !device->category)
		return;

	memcpy(&devices[device_count], device, sizeof(device_t));
	memcpy(&devices[device_count].name, name, strlen(name) + 1);

	kprintf("devmgr: registered %s device: %s\n", device_types[device->category], name);
	device_count++;
}

// devmgr_dump(): Dumps all devices
// Param:	Nothing
// Return:	Nothing

void devmgr_dump()
{
	kprintf("devmgr: dumping all present devices:\n");

	size_t i = 0;
	size_t k = 0;
	while(i < device_count)
	{
		k = 0;

		kprintf(" Device %d: %s device: %s\n", i, device_types[devices[i].category], &devices[i].name);
		if(devices[i].irq != 0)
			kprintf("  IRQ line: %d\n", devices[i].irq);

		while(k < MAX_DEVICE_ADDRESS)
		{
			if(devices[i].mmio[k].size == 0)
				break;

			kprintf("  MMIO: 0x%xq - 0x%xq\n", devices[i].mmio[k].base, devices[i].mmio[k].base + devices[i].mmio[k].size);
			k++;
		}

		k = 0;
		while(k < MAX_DEVICE_ADDRESS)
		{
			if(devices[i].io[k].size == 0)
				break;

			kprintf("  I/O port: 0x%xw - 0x%xw\n", devices[i].io[k].base, devices[i].io[k].base + devices[i].io[k].size);
			k++;
		}

		i++;
	}
}


