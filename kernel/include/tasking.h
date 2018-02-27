
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define MAX_PROCESSES			512

#define PROCESS_FLAGS_PRESENT		0x01
#define PROCESS_FLAGS_BLOCKED		0x02
#define PROCESS_FLAGS_ACTIVE		0x80

#define PROCESS_TIMESLICE		5

#if __i386__
typedef struct process_t
{
	uint32_t eip;
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eflags;

	uint8_t flags;
	uint8_t time;

	size_t pmem_base;
	size_t pmem_size;
	size_t tty;

	char path[1024];
} process_t;
#endif

#if __x86_64__
typedef struct process_t
{
	uint64_t rip;
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rflags;

	uint8_t flags;
	uint8_t time;

	size_t pmem_base;
	size_t pmem_size;
	size_t tty;

	char path[1024];
} process_t;
#endif

process_t *processes;

void tasking_init();
char *get_path(char *);
pid_t get_pid();
size_t get_tty();


