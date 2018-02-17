
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <apic.h>

#if __i386__
// NULL, KCODE32, KDATA32, UCODE32, UDATA32, one segment for each CPU + one TSS for each CPU
#define GDT_ENTRIES			(5 + MAX_LAPICS + MAX_LAPICS)
#define GDT_CPU_INFO			5		// starting at index 5
#define GDT_TSS				(GDT_CPU_INFO + MAX_LAPICS)
#endif

#if __x86_64__
// NULL, KCODE32, KDATA32, KCODE64, KDATA64, UCODE64, UDATA64, UCODE32, UDATA32 + one TSS for each CPU
// in 64-bit long mode, we don't need a segment for each AP because we can use
// MSR_FS_BASE for CPU-specific information
#define GDT_ENTRIES			(9 + MAX_LAPICS + MAX_LAPICS)	// each TSS takes two entries in long mode
#define GDT_TSS				9
#endif

// GDT Access Byte
#define GDT_ACCESS_RW			0x02
#define GDT_ACCESS_DC			0x04
#define GDT_ACCESS_EXEC			0x08
#define GDT_ACCESS_RING_SHIFT		5
#define GDT_ACCESS_PRESENT		0x80

// GDT Flags
#define GDT_FLAGS_LONG_MODE		0x20
#define GDT_FLAGS_PMODE			0x40
#define GDT_FLAGS_PAGE_GRANULARITY	0x80

typedef struct gdt_t
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
}__attribute__((packed)) gdt_t;

typedef struct gdtr_t
{
	uint16_t limit;
	uint64_t base;
}__attribute__((packed)) gdtr_t;

gdt_t *gdt;
gdtr_t *gdtr;

void gdt_init();
void gdt_set_entry(size_t, uint32_t, uint8_t, uint8_t);
void gdt_set_limit(size_t, uint32_t);
void flush_gdt(gdtr_t *, uint16_t, uint16_t);



