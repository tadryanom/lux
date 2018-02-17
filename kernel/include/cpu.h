
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define STACK_SIZE		65536		// kernel stack

// These attributes work with clang only, I think
#define GS_BASE			__attribute__((address_space(256)))
#define FS_BASE			__attribute__((address_space(257)))

#if __x86_64__

// x86_64 Model Specific Registers
#define MSR_FS_BASE		0xC0000100
#define MSR_GS_BASE		0xC0000101
#define MSR_KERNEL_GS_BASE	0xC0000102	// swapgs instruction

#endif

typedef struct cpu_t
{
	size_t index;
	void *stack;
	uint64_t timestamp;
	size_t process_count;
	pid_t current_pid;
} cpu_t;

#if __i386__
extern void write_cr0(uint32_t);
extern void write_cr3(uint32_t);
extern void write_cr4(uint32_t);

extern uint32_t read_cr0();
extern uint32_t read_cr2();
extern uint32_t read_cr3();
extern uint32_t read_cr4();
extern void load_fs(uint16_t);
#endif

#if __x86_64__
extern void write_cr0(uint64_t);
extern void write_cr3(uint64_t);
extern void write_cr4(uint64_t);

extern uint64_t read_cr0();
extern uint64_t read_cr2();
extern uint64_t read_cr3();
extern uint64_t read_cr4();

extern void write_msr(uint32_t, uint64_t);
extern uint64_t read_msr(uint32_t);
#endif

extern void flush_tlb(size_t, size_t);




