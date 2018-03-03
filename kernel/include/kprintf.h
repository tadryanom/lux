
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

char debug_mode;

void kprint_init();
void kprintf(char *, ...);
int sprintf(char *, const char *, ...);

void install_exceptions();
void panic(char *);
void exception_handler(char *, uint32_t);

void save_registers(registers_t *);
void dump_registers(registers_t *);

extern void divide_handler();
extern void debug_handler();
extern void nmi_handler();
extern void breakpoint_handler();
extern void overflow_handler();
extern void bound_handler();
extern void opcode_handler();
extern void device_handler();
extern void double_handler();
extern void coprocessor_handler();
extern void tss_handler();
extern void segment_handler();
extern void stack_handler();
extern void gpf_handler();
extern void page_handler();
extern void reserved_handler();
extern void floating_handler();
extern void alignment_handler();
extern void machine_handler();
extern void simd_handler();
extern void virtual_handler();
extern void security_handler();



