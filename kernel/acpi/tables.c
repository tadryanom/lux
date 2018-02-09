
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <kprintf.h>
#include <acpi.h>
#include <mm.h>
#include <string.h>

acpi_rsdp_t *rsdp;
acpi_rsdt_t *rsdt;

// acpi_init(): Detects and initializes ACPI
// Param:	Nothing
// Return:	Nothing

void acpi_init()
{
	// search for the RSDP -- 'RSD PTR ' should be in low memory aligned on 16 bytes
	char *scan_ptr = (char*)0xE0000;

	while(1)
	{
		if(memcmp(scan_ptr, "RSD PTR ", 8) == 0)
		{
			rsdp = (acpi_rsdp_t*)scan_ptr;
			break;
		}
		else
		{
			scan_ptr += 16;
			if(scan_ptr >= (char*)0xFFFF0)
			{
				kprintf("acpi: boot failed, system doesn't support ACPI.\n");
				while(1);
			}
		}
	}

	// we won't verify the RSDP checksum
	// but we will for each table that we use
	kprintf("acpi: 'RSD PTR ' 0x%xd v%xb\n", (uint32_t)rsdp, rsdp->revision);

	rsdt = (acpi_rsdt_t*)vmm_request_map(rsdp->rsdt, 4, PAGE_PRESENT | PAGE_RW);

	if(acpi_checksum(rsdt) != 0)
	{
		kprintf("acpi: boot failed, 'RSDT' checksum failed.\n");
		while(1);
	}

	kprintf("acpi: 'RSDT' 0x%xd len %d v%xb OEM '%c%c%c%c%c%c'\n", rsdp->rsdt, rsdt->header.length, rsdt->header.revision, rsdt->header.oem[0], rsdt->header.oem[1], rsdt->header.oem[2], rsdt->header.oem[3], rsdt->header.oem[4], rsdt->header.oem[5]);

	// show each table
	size_t rsdt_count = (rsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint32_t);
	size_t i = 0;
	acpi_header_t *header;

	while(i < rsdt_count)
	{
		header = (acpi_header_t*)vmm_request_map(rsdt->tables[i], 1, PAGE_PRESENT | PAGE_RW);
		kprintf("acpi: '%c%c%c%c' 0x%xd len %d v%xb OEM '%c%c%c%c%c%c'\n", header->signature[0], header->signature[1], header->signature[2], header->signature[3], rsdt->tables[i], header->length, header->revision, header->oem[0], header->oem[1], header->oem[2], header->oem[3], header->oem[4], header->oem[5]);

		vmm_unmap((size_t)header, 1);
		i++;
	}
}

// acpi_checksum(): Verifies checksum of a table
// Param:	void * - pointer to table
// Return:	int - 0 on success

int acpi_checksum(void *ptr)
{
	acpi_header_t *header = (acpi_header_t*)ptr;
	uint8_t *bytes = (uint8_t*)ptr;
	uint8_t *end_bytes = bytes + header->length;

	uint8_t val = 0;

	while(bytes < end_bytes)
	{
		val += bytes[0];
		bytes++;
	}

	if(val == 0)
		return 0;

	else
	{
		kprintf("acpi: checksum failed for table '%c%c%c%c', not allowing this table to be used.\n", header->signature[0], header->signature[1], header->signature[2], header->signature[3]);
		return 1;
	}
}

// acpi_scan(): Scans for an ACPI table
// Param:	char *signature - signautre of table
// Param:	size_t index - index for those that have more than one
// Return:	void * - pointer to table, NULL on error
//
// NOTE:
// the "index" exists because there can be more than one table with the same
// name, the most common example being the 'SSDT' on real hardware when there
// there are more than one 'SSDT's, they are all nescessary and complete each
// other's AML code, so this parameter is nescessary

void *acpi_scan(char *signature, size_t index)
{
	if(strlen(signature) != 4)
		return NULL;

	size_t count = 0;

	size_t rsdt_count = (rsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint32_t);
	size_t i = 0;
	size_t pages;
	acpi_header_t *header;

	while(1)
	{
		header = (acpi_header_t*)vmm_request_map(rsdt->tables[i], 1, PAGE_PRESENT | PAGE_RW);
		if(memcmp(header->signature, signature, 4) == 0)
		{
			if(count == index)
				break;
			else
				count++;
		}

		vmm_unmap((size_t)header, 1);
		i++;
		if(i >= rsdt_count)
		{
			kprintf("acpi: table '%s' index %d doesn't exist.\n", signature, index);
			return NULL;
		}
	}

	// map the table in virtual memory
	pages = (header->length + PAGE_SIZE - 1) / PAGE_SIZE;
	void *ptr = (void*)vmm_request_map(rsdt->tables[i], pages, PAGE_PRESENT | PAGE_RW);
	vmm_unmap((size_t)header, 1);

	// and verify the checksum before returning
	if(acpi_checksum(ptr) == 0)
		return ptr;
	else
	{
		vmm_unmap((size_t)ptr, pages);
		return NULL;
	}
}





