
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define STACK_SIZE		65536		// kernel stack

typedef struct cpu_t
{
	uint8_t index;
	void *stack;
} cpu_t;

cpu_t *cpus;

#if __i386__
extern void write_cr0(uint32_t);
extern void write_cr3(uint32_t);
extern void write_cr4(uint32_t);

extern uint32_t read_cr0();
extern uint32_t read_cr2();
extern uint32_t read_cr3();
extern uint32_t read_cr4();
#endif

#if __x86_64__
extern void write_cr0(uint64_t);
extern void write_cr3(uint64_t);
extern void write_cr4(uint64_t);

extern uint64_t read_cr0();
extern uint64_t read_cr2();
extern uint64_t read_cr3();
extern uint64_t read_cr4();
#endif

extern void flush_tlb(size_t, size_t);




