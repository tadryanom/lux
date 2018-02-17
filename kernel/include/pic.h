
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

void pic_init(uint8_t);
void pic_mask(uint8_t);
void pic_unmask(uint8_t);
void pic_eoi(uint8_t);

