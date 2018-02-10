
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <acpi.h>

#define IRQ_BASE		0x30

// Limitations
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
#define IOAPIC_ID		0
#define IOAPIC_VER		1
#define IOAPIC_ARBITRATION	2
#define IOAPIC_IRQ_TABLE	16

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

void apic_init();
void apic_parse();

void apic_register_lapic(madt_lapic_t *);
void apic_register_ioapic(madt_ioapic_t *);
void apic_register_override(madt_override_t *);

uint32_t ioapic_read(size_t, uint32_t);
void ioapic_write(size_t, uint32_t, uint32_t);




