
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#define NULL			0

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

#if __x86_64__
typedef uint64_t size_t;
#endif

#if __i386__
typedef uint32_t size_t;
#endif

extern void *kend;

#if __i386__
typedef struct registers_t
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eflags;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t es;
	uint32_t cr0;
	uint32_t cr2;
	uint32_t cr3;
	uint32_t cr4;
}__attribute__((packed)) registers_t;
#endif

#if __x86_64__
typedef struct registers_t
{
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
	uint64_t cs;
	uint64_t ss;
	uint64_t ds;
	uint64_t es;
	uint64_t cr0;
	uint64_t cr2;
	uint64_t cr3;
	uint64_t cr4;
}__attribute__((packed)) registers_t;
#endif


