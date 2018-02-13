
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <idt.h>

// idt_install(): Installs an interrupt handler
// Param:	uint8_t interrupt - interrupt number
// Param:	uint32_t handler - address of handler
// Return:	Nothing

void idt_install(uint8_t interrupt, uint32_t handler)
{
#if __i386__
	idt[interrupt].handler_low = (uint16_t)handler & 0xFFFF;
	idt[interrupt].handler_high = (uint16_t)(handler >> 16) & 0xFFFF;
#endif
}

// idt_set_flags(): Sets flags of an interrupt
// Param:	uint8_t interrupt - interrupt number
// Param:	uint16_t flags - flags value
// Return:	Nothing

void idt_set_flags(uint8_t interrupt, uint16_t flags)
{
#if __i386__
	idt[interrupt].flags = flags;
#endif
}



