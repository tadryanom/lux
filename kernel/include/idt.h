
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#if __i386__
typedef struct idt_t
{
	uint16_t handler_low;
	uint16_t segment;
	uint8_t reserved;
	uint8_t flags;
	uint16_t handler_high;
	
}__attribute__((packed)) idt_t;
#endif

#if __x86_64__
typedef struct idt_t
{
	uint16_t handler_low;
	uint16_t segment;
	uint8_t interrupt_stack;
	uint8_t flags;
	uint16_t handler_middle;
	uint32_t handler_high;
	uint32_t reserved;
}__attribute__((packed)) idt_t;
#endif

extern idt_t idt[];

void idt_install(uint8_t interrupt, size_t handler);
void idt_set_flags(uint8_t interrupt, uint8_t flags);




