
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

void iowait();

void outb(uint16_t, uint8_t);
void outw(uint16_t, uint16_t);
void outd(uint16_t, uint32_t);

uint8_t inb(uint16_t);
uint16_t inw(uint16_t);
uint32_t ind(uint16_t);



