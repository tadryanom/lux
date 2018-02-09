
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

typedef struct acpi_rsdp_t
{
	char signature[8];
	uint8_t checksum;
	char oem[6];
	uint8_t revision;
	uint32_t rsdt;
	uint32_t length;
	uint64_t xsdt;
	uint8_t extended_checksum;
}__attribute__((packed)) acpi_rsdp_t;

typedef struct acpi_header_t
{
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem[6];
	char oem_table[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
}__attribute__((packed)) acpi_header_t;

typedef struct acpi_rsdt_t
{
	acpi_header_t header;
	uint32_t tables[];
}__attribute__((packed)) acpi_rsdt_t;

void acpi_init();
int acpi_checksum(void *);
void *acpi_scan(char *, size_t);




