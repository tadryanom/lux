
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

void irq_eoi(uint8_t);
void irq_mask(uint8_t);
void irq_unmask(uint8_t);
void irq_install(uint8_t, size_t);
