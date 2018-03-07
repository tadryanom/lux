
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <mm.h>
#include <kprintf.h>
#include <io.h>

acpi_fadt_t *fadt;
acpi_aml_t *dsdt;

// acpi_load_fadt(): Loads the FADT and DSDT tables
// Param:	Nothing
// Return:	Nothing

void acpi_load_fadt()
{
	fadt = acpi_scan("FACP", 0);
	if(!fadt)
	{
		kprintf("acpi: boot failed, FADT table not present.\n");
		while(1);
	}

	uint64_t dsdt_physical;

	dsdt = (acpi_aml_t*)((size_t)fadt->dsdt);

	dsdt_physical = (uint64_t)dsdt;

	acpi_header_t *dsdt_header = (acpi_header_t*)vmm_request_map((size_t)dsdt, 1, PAGE_PRESENT | PAGE_RW);
	dsdt = (acpi_aml_t*)vmm_request_map((size_t)dsdt, (size_t)(dsdt_header->length + PAGE_SIZE - 1) / PAGE_SIZE, PAGE_PRESENT | PAGE_RW);
	vmm_unmap((size_t)dsdt_header, 1);

	// do the checksum
	if(acpi_checksum(dsdt) != 0)
	{
		kprintf("acpi: boot failed, DSDT checksum error.\n");
		while(1);
	}

	kprintf("acpi: 'DSDT' 0x%xq len %d v%xb OEM '%c%c%c%c%c%c'\n", dsdt_physical, dsdt->header.length, dsdt->header.revision, dsdt->header.oem[0], dsdt->header.oem[1], dsdt->header.oem[2], dsdt->header.oem[3], dsdt->header.oem[4], dsdt->header.oem[5]);
}




