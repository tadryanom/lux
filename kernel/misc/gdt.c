
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <gdt.h>
#include <kprintf.h>
#include <mm.h>

gdt_t *gdt;
gdtr_t *gdtr;

// gdt_init(): Creates a GDT
// Param:	Nothing
// Return:	Nothing

void gdt_init()
{
	gdt = kcalloc(sizeof(gdt_t), GDT_ENTRIES);
	gdtr = kcalloc(sizeof(gdtr_t), 1);

	gdtr->limit = (sizeof(gdt_t) * GDT_ENTRIES) - 1;
	gdtr->base = (uint64_t)gdt;

#if __i386__
	// 32-bit kernel code/data segments
	gdt_set_entry(1, 0, GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(2, 0, GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);

	// 32-bit user code/data segments
	gdt_set_entry(3, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(4, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);

	flush_gdt(gdtr, 0x08, 0x10);
#endif

#if __x86_64__
	// 32-bit kernel code/data segments
	gdt_set_entry(1, 0, GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(2, 0, GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);

	// 64-bit kernel code/data segments
	gdt_set_entry(3, 0, GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_LONG_MODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(4, 0, GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_LONG_MODE | GDT_FLAGS_PAGE_GRANULARITY);

	// 64-bit user code/data segments
	gdt_set_entry(5, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_LONG_MODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(6, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_LONG_MODE | GDT_FLAGS_PAGE_GRANULARITY);

	// 32-bit user code/data segments
	gdt_set_entry(7, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_EXEC | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);
	gdt_set_entry(8, 0, (3 << GDT_ACCESS_RING_SHIFT) | GDT_ACCESS_RW | GDT_ACCESS_PRESENT, GDT_FLAGS_PMODE | GDT_FLAGS_PAGE_GRANULARITY);

	flush_gdt(gdtr, 0x18, 0x20);
#endif
}

// gdt_set_entry(): Writes a GDT entry
// Param:	size_t entry - index in the GDT
// Param:	uint32_t base - segment base
// Param:	uint8_t access - access byte
// Param:	uint8_t flags - flags nibble in higher nibble
// Return:	Nothing

void gdt_set_entry(size_t entry, uint32_t base, uint8_t access, uint8_t flags)
{
	gdt[entry].limit_low = 0xFFFF;
	gdt[entry].base_low = (uint16_t)base & 0xFFFF;
	gdt[entry].base_middle = (uint8_t)(base >> 16) & 0xFF;
	gdt[entry].access = access | 0x10;	// bit 4 is always set
	gdt[entry].flags = flags | 0x0F;	// limit high
	gdt[entry].base_high = (uint8_t)(base >> 24) & 0xFF;

	/*kprintf("gdt[%d]: limit_lo = %xw\n", entry, gdt[entry].limit_low);
	kprintf("gdt[%d]: base_lo = %xw\n", entry, gdt[entry].base_low);
	kprintf("gdt[%d]: base_middle = %xb\n", entry, gdt[entry].base_middle);
	kprintf("gdt[%d]: access = %xb\n", entry, gdt[entry].access);
	kprintf("gdt[%d]: flags = %xb\n", entry, gdt[entry].flags);
	kprintf("gdt[%d]: base_high = %xb\n", entry, gdt[entry].base_high);*/
}

// gdt_set_limit(): Sets limit of a GDT entry
// Param:	size_t entry - index in the GDT
// Param:	uint32_t limit - limit, only 20 bits are used
// Return:	Nothing

void gdt_set_limit(size_t entry, uint32_t limit)
{
	gdt[entry].limit_low = (uint16_t)limit & 0xFFFF;
	gdt[entry].flags &= 0xF0;
	gdt[entry].flags |= (uint8_t)((limit >> 16) & 0x0F);
	gdt[entry].flags &= (~GDT_FLAGS_PAGE_GRANULARITY);
}


