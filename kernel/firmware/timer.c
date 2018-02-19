
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <timer.h>
#include <kprintf.h>
#include <cpu.h>
#include <irq.h>
#include <lock.h>

uint64_t global_uptime = 0;
uint8_t timer_irq_line;

// timer_init(): Initializes timers
// Param:	Nothing
// Return:	Nothing

void timer_init()
{
	pit_init();
	//hpet_init();
}

// timer_irq(): Generic timer IRQ handler
// Param:	Nothing
// Return:	Nothing

void timer_irq()
{
	cpu_t FS_BASE *cpu = (cpu_t FS_BASE*)0;
	cpu->timestamp++;

	if(cpu->index == 0)
		global_uptime = cpu->timestamp;

	irq_eoi(timer_irq_line);
}




