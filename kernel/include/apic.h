
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <acpi.h>

// We'll use INT 0x30 as a base for hardware IRQs
// Because if we use the I/O APIC, the legacy PIC can still make spurious IRQs
// so we need to handle those, so we'll use base 0x20 for the PIC when we use
// the I/O APIC -- if we use the PIC we'll still use base 0x30

#define IRQ_BASE		0x30
#define UNUSED_PIC_BASE		0x20

// Limitations -- don't try to increase these --
// -- because we have no xAPIC support.
#define MAX_LAPICS		16		// # of CPUs
#define MAX_IOAPICS		16
#define MAX_OVERRIDES		48

// To Determine Which Interrupt Controller We're Using
#define IRQ_IOAPIC		1
#define IRQ_PIC			2

// ACPI MADT Table Fields
#define MADT_ENTRY_LAPIC	0
#define MADT_ENTRY_IOAPIC	1
#define MADT_ENTRY_OVERRIDE	2
#define MADT_IRQ_ACTIVE_LOW	0x0002
#define MADT_IRQ_LEVEL		0x0008

// I/O APIC Registers
#define IOAPIC_ID		0x00
#define IOAPIC_VER		0x01
#define IOAPIC_ARBITRATION	0x02
#define IOAPIC_IRQ_TABLE	0x10

#define IOAPIC_ACTIVE_LOW	0x2000
#define IOAPIC_LEVEL		0x8000
#define IOAPIC_MASK		0x10000

// Local APIC Registers
#define LAPIC_ID		0x020
#define LAPIC_VERSION		0x030
#define LAPIC_TASK_PRIORITY	0x080
#define LAPIC_EOI		0x0B0
#define LAPIC_SPURIOUS_IRQ	0x0F0
#define LAPIC_COMMAND		0x300
#define LAPIC_COMMAND_ID	0x310
#define LAPIC_TIMER_INIT_COUNT	0x380
#define LAPIC_TIMER_CURR_COUNT	0x390
#define LAPIC_TIMER_DIVIDE	0x3E0

// Structures...

typedef struct acpi_madt_t
{
	acpi_header_t header;
	uint32_t local_apic;
	uint32_t flags;

	uint8_t records[];
}__attribute__((packed)) acpi_madt_t;

typedef struct madt_lapic_t
{
	uint8_t type;
	uint8_t size;
	uint8_t acpi_id;
	uint8_t apic_id;
	uint32_t flags;
}__attribute__((packed)) madt_lapic_t;

typedef struct madt_ioapic_t
{
	uint8_t type;
	uint8_t size;
	uint8_t apic_id;
	uint8_t reserved;
	uint32_t base;
	uint32_t gsi;
}__attribute__((packed)) madt_ioapic_t;

typedef struct madt_override_t
{
	uint8_t type;
	uint8_t size;
	uint8_t bus;
	uint8_t irq;
	uint32_t gsi;
	uint16_t flags;
}__attribute__((packed)) madt_override_t;

typedef struct lapic_t
{
	uint8_t present;
	uint8_t apic_id;
} lapic_t;

typedef struct ioapic_t
{
	uint8_t present;
	uint8_t apic_id;
	uint32_t base_phys;
	size_t base;
	uint32_t gsi;
	uint32_t irq_count;
} ioapic_t;

typedef struct irq_override_t
{
	uint8_t present;
	uint8_t irq;
	uint8_t bus;
	uint32_t gsi;
	uint16_t flags;
} irq_override_t;

acpi_madt_t *madt;
char irq_mode;

lapic_t *lapics;
ioapic_t *ioapics;
irq_override_t *overrides;
size_t lapic_count, ioapic_count, override_count;

void *lapic_base;

void apic_init();
void apic_parse();

void apic_register_lapic(madt_lapic_t *);
void apic_register_ioapic(madt_ioapic_t *);
void apic_register_override(madt_override_t *);

uint32_t ioapic_read(size_t, uint32_t);
void ioapic_write(size_t, uint32_t, uint32_t);
void ioapic_init();
void ioapic_mask(uint8_t);
void ioapic_unmask(uint8_t);

uint32_t lapic_read(size_t);
void lapic_write(size_t, uint32_t);
void lapic_eoi();

void smp_init();
extern char trampoline16[];
extern uint16_t trampoline16_size[];




