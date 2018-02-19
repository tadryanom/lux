
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define IRQ_ACTIVE_LOW		0x01
#define IRQ_ACTIVE_HIGH		0x00
#define IRQ_LEVEL		0x02
#define IRQ_EDGE		0x00
#define IRQ_BROADCAST		0x80		// for I/O APIC only

void irq_eoi(uint8_t);
void irq_mask(uint8_t);
void irq_unmask(uint8_t);
void irq_install(uint8_t, size_t);
uint8_t irq_configure(uint8_t, uint8_t);



