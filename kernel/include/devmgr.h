
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define MAX_DEVICES			1024
#define MAX_DEVICE_ADDRESS		8

#define DEVMGR_CATEGORY_SYSTEM		1
#define DEVMGR_CATEGORY_CPU		2
#define DEVMGR_CATEGORY_GRAPHICS	3
#define DEVMGR_CATEGORY_NETWORK		4
#define DEVMGR_CATEGORY_DISK_CONTROLLER	5
#define DEVMGR_CATEGORY_DISK		6
#define DEVMGR_CATEGORY_KEYBOARD	7
#define DEVMGR_CATEGORY_MOUSE		8
#define DEVMGR_CATEGORY_USB_CONTROLLER	9
#define DEVMGR_CATEGORY_USB_DEVICE	10
#define DEVMGR_CATEGORY_COMM		11

#define DEVMGR_LAST_CATEGORY		11

typedef struct device_io_t
{
	uint16_t base;
	uint16_t size;
} device_io_t;

typedef struct device_mmio_t
{
	uint64_t base;
	uint64_t size;
} device_mmio_t;

typedef struct device_t
{
	uint8_t category;
	uint8_t irq;

	device_io_t io[MAX_DEVICE_ADDRESS];
	device_mmio_t mmio[MAX_DEVICE_ADDRESS];

	char name[128];
} device_t;

void devmgr_init();
void devmgr_register(device_t *, char *);
void devmgr_dump();


