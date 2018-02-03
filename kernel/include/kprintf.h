
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

void kprint_init();
void kprintf(char *, ...);
void panic(char *);

void save_registers(registers_t *);
void dump_registers(registers_t *);
