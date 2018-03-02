
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define TIMER_FREQUENCY			1000	// Hz

extern void timer_irq_stub();

uint64_t global_uptime;
uint8_t timer_irq_line;

void timer_init();
void pit_init();



