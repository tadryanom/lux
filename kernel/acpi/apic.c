
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <apic.h>
#include <pic.h>
#include <mm.h>
#include <kprintf.h>
#include <devmgr.h>
#include <irq.h>

acpi_madt_t *madt;
char irq_mode = 0;

lapic_t *lapics;
ioapic_t *ioapics;
irq_override_t *overrides;

size_t lapic_count, ioapic_count, override_count;

// apic_init(): Initializes SMP and I/O APICs
// Param:	Nothing
// Return:	Nothing

void apic_init()
{
	lapics = kcalloc(sizeof(lapic_t), MAX_LAPICS);
	ioapics = kcalloc(sizeof(ioapic_t), MAX_IOAPICS);
	overrides = kcalloc(sizeof(irq_override_t), MAX_OVERRIDES);
	lapic_count = 0;
	ioapic_count = 0;
	override_count = 0;

	// find ACPI MADT table
	madt = acpi_scan("APIC", 0);
	if(!madt)
	{
		kprintf("apic: ACPI MADT not present, using one CPU and legacy PIC for IRQs.\n");
		irq_mode = IRQ_PIC;
		pic_init(IRQ_BASE);

		// we have no APICs, but at least pretend we have a BSP local APIC
		lapics[0].present = 1;
		lapics[0].apic_id = 0;
		lapic_count = 1;
		smp_register_cpu(0);
	}

	apic_parse();
	smp_init();

	if(ioapic_count != 0)
	{
		irq_mode = IRQ_IOAPIC;
		ioapic_init();
	}
	else
	{
		irq_mode = IRQ_PIC;
		pic_init(IRQ_BASE);
	}

	// we can now enable interrupts
	asm volatile ("sti");
}

// apic_parse(): Parses the APIC table data
// Param:	Nothing
// Return:	Nothing

void apic_parse()
{
	kprintf("apic: local APIC is at 0x%xd\n", madt->local_apic);
	kprintf("apic: MADT flags = 0x%xd\n", madt->flags);

	// register the local APIC
	device_t *lapic_device = kmalloc(sizeof(device_t));
	lapic_device->category = DEVMGR_CATEGORY_SYSTEM;
	lapic_device->mmio[0].base = (uint64_t)madt->local_apic;
	lapic_device->mmio[0].size = (uint64_t)0xFFF;	// question: is the local APIC MMIO always one page long?
	devmgr_register(lapic_device, "Local APIC");

	kfree(lapic_device);

	lapic_base = (void*)vmm_request_map(madt->local_apic, 2, PAGE_PRESENT | PAGE_RW | PAGE_UNCACHEABLE);

	char *ptr = (char*)madt->records;
	uint32_t bytes = 0;

	madt_lapic_t *lapic;
	madt_ioapic_t *ioapic;
	madt_override_t *override;

	while(bytes < madt->header.length - 0x2C)
	{
		if(ptr[1] <= 2)
			break;

		switch(ptr[0])
		{
		case MADT_ENTRY_LAPIC:
			lapic = (madt_lapic_t*)ptr;
			apic_register_lapic(lapic);
			break;

		case MADT_ENTRY_IOAPIC:
			ioapic = (madt_ioapic_t*)ptr;
			apic_register_ioapic(ioapic);
			break;

		case MADT_ENTRY_OVERRIDE:
			override = (madt_override_t*)ptr;
			apic_register_override(override);
			break;

		default:
			kprintf("apic: unknown entry type 0x%xb size %d, ignoring...\n", ptr[0], ptr[1]);
			break;
		}

		ptr += ptr[1];
		bytes += ptr[1];
	}
}

// apic_register_lapic(): Registers a CPU local APIC
// Param:	madt_lapic_t *data - data from MADT table
// Return:	Nothing

void apic_register_lapic(madt_lapic_t *data)
{
	kprintf("apic: CPU local APIC ID 0x%xb flags 0x%xd\n", data->apic_id, data->flags);

	if(lapic_count >= MAX_LAPICS)
		return;

	if(!data->flags & 1)
		return;

	lapics[lapic_count].present = 1;
	lapics[lapic_count].apic_id = data->apic_id;
	lapic_count++;
}

// apic_register_ioapic(): Registers an I/O APIC
// Param:	madt_ioapic_t *data - data from MADT table
// Return:	Nothing

void apic_register_ioapic(madt_ioapic_t *data)
{
	if(ioapic_count >= MAX_IOAPICS)
		return;

	ioapics[ioapic_count].present = 1;
	ioapics[ioapic_count].apic_id = data->apic_id;
	ioapics[ioapic_count].base_phys = data->base;
	ioapics[ioapic_count].gsi = data->gsi;

	// map the I/O APIC -- uncacheable because hardware MMIO
	ioapics[ioapic_count].base = vmm_request_map(data->base, 1, PAGE_PRESENT | PAGE_RW | PAGE_UNCACHEABLE);

	// now read from the I/O APIC to determine how many IRQs it can handle
	ioapics[ioapic_count].irq_count = (uint32_t)(ioapic_read(ioapic_count, IOAPIC_VER) >> 16) & 0xFF;

	kprintf("apic: I/O APIC ID 0x%xb MMIO 0x%xd GSI %d IRQ count %d\n", data->apic_id, data->base, data->gsi, ioapics[ioapic_count].irq_count);
	ioapic_count++;
}

// apic_register_override(): Registers an IRQ override
// Param:	madt_override_t *data - data from MADT table
// Return:	Nothing

void apic_register_override(madt_override_t *data)
{
	if(override_count >= MAX_OVERRIDES)
		return;

	overrides[override_count].present = 1;
	overrides[override_count].bus = data->bus;
	overrides[override_count].irq = data->irq;
	overrides[override_count].gsi = data->gsi;
	//overrides[override_count].flags = data->flags;

	if(data->flags & MADT_IRQ_ACTIVE_LOW)
		overrides[override_count].flags = IRQ_ACTIVE_LOW;
	else
		overrides[override_count].flags = IRQ_ACTIVE_HIGH;

	if(data->flags & MADT_IRQ_LEVEL)
		overrides[override_count].flags |= IRQ_LEVEL;
	else
		overrides[override_count].flags |= IRQ_EDGE;

	kprintf("apic: override IRQ %d bus %d GSI %d flags 0x%xw (", data->irq, data->bus, data->gsi, data->flags);

	if(data->flags & MADT_IRQ_LEVEL)
		kprintf("level ");
	else
		kprintf("edge ");

	if(data->flags & MADT_IRQ_ACTIVE_LOW)
		kprintf("low");
	else
		kprintf("high");

	kprintf(")\n");

	override_count++;
}




