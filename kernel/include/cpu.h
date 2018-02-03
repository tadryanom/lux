
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <types.h>

extern void write_cr0(uint32_t);
extern void write_cr3(uint32_t);
extern void write_cr4(uint32_t);

extern uint32_t read_cr0();
extern uint32_t read_cr2();
extern uint32_t read_cr3();
extern uint32_t read_cr4();

extern void flush_tlb(size_t, size_t);



