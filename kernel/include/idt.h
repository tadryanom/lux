
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

typedef struct idt_t
{
	uint16_t handler_low;
	uint16_t segment;
	uint16_t flags;
	uint16_t handler_high;
}__attribute__((packed)) idt_t;

extern idt_t idt[];

void idt_install(uint8_t interrupt, uint32_t handler);
void idt_set_flags(uint8_t interrupt, uint16_t flags);




