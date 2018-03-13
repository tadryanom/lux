
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <lock.h>

#define ACPI_GAS_MMIO			0
#define ACPI_GAS_IO			1
#define ACPI_GAS_PCI			2

#define ACPI_MAX_NAMESPACE_ENTRIES	128		// realloc()'d, to save memory
#define ACPI_MAX_PACKAGE_ENTRIES	256		// Package(), VarPackage() is unlimited

#define ACPI_NAMESPACE_NAME		1
#define ACPI_NAMESPACE_ALIAS		2
#define ACPI_NAMESPACE_SCOPE		3
#define ACPI_NAMESPACE_FIELD		4
#define ACPI_NAMESPACE_METHOD		5
#define ACPI_NAMESPACE_DEVICE		6
#define ACPI_NAMESPACE_INDEXFIELD	7
#define ACPI_NAMESPACE_MUTEX		8
#define ACPI_NAMESPACE_PROCESSOR	9
#define ACPI_NAMESPACE_BUFFER_FIELD	10

#define ACPI_INTEGER			1
#define ACPI_STRING			2
#define ACPI_PACKAGE			3
#define ACPI_BUFFER			4
#define ACPI_NAME			5

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

typedef struct acpi_gas_t
{
	uint8_t address_space;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t base;
}__attribute__((packed)) acpi_gas_t;

typedef struct acpi_fadt_t
{
	acpi_header_t header;
	uint32_t firmware_control;
	uint32_t dsdt;		// pointer to dsdt

	uint8_t reserved;

	uint8_t profile;
	uint16_t sci_irq;
	uint32_t smi_command_port;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_control;
	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
	uint32_t pm2_control_block;
	uint32_t pm_timer_block;
	uint32_t gpe0_block;
	uint32_t gpe1_block;
	uint8_t pm1_event_length;
	uint8_t pm1_control_length;
	uint8_t pm2_control_length;
	uint8_t pm_timer_length;
	uint8_t gpe0_length;
	uint8_t gpe1_length;
	uint8_t gpe1_base;
	uint8_t cstate_control;
	uint16_t worst_c2_latency;
	uint16_t worst_c3_latency;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;

	// cmos registers
	uint8_t day_alarm;
	uint8_t month_alarm;
	uint8_t century;

	// ACPI 2.0 fields
	uint16_t boot_flags;
	uint8_t reserved2;
	uint32_t flags;

	acpi_gas_t reset_register;
	uint8_t reset_command;
	uint8_t reserved3[3];

	uint64_t x_firmware_control;
	uint64_t x_dsdt;

	acpi_gas_t x_pm1a_event_block;
	acpi_gas_t x_pm1b_event_block;
	acpi_gas_t x_pm1a_control_block;
	acpi_gas_t x_pm1b_control_block;
	acpi_gas_t x_pm2_control_block;
	acpi_gas_t x_pm_timer_block;
	acpi_gas_t x_gpe0_block;
	acpi_gas_t x_gpe1_block;
}__attribute__((packed)) acpi_fadt_t;

typedef struct acpi_aml_t		// AML tables, DSDT and SSDT
{
	acpi_header_t header;
	uint8_t data[];
}__attribute__((packed)) acpi_aml_t;

typedef struct acpi_object_t
{
	int type;
	uint64_t integer;		// for Name()
	char *string;			// for Name()

	int package_size;		// for Package(), size in entries
	struct acpi_object_t *package;	// for Package(), actual entries

	size_t buffer_size;		// for Buffer(), size in bytes
	void *buffer;			// for Buffer(), actual bytes

	char name[512];			// for Name References
} acpi_object_t;

typedef struct acpi_handle_t
{
	char path[512];			// full path of object
	int type;
	void *pointer;			// valid for scopes, methods, etc.
	size_t size;			// valid for scopes, methods, etc.

	char alias[512];		// for Alias() only
	acpi_object_t object;		// for Name()

	uint8_t op_address_space;	// for OpRegions only
	uint64_t op_base;		// for OpRegions only
	uint64_t op_length;		// for OpRegions only

	uint64_t field_offset;		// for Fields only, in bits
	uint8_t field_size;		// for Fields only, in bits
	uint8_t field_flags;		// for Fields only
	char field_opregion[512];	// for Fields only

	uint8_t method_flags;		// for Methods only, includes ARG_COUNT in lowest three bits

	uint64_t indexfield_offset;	// for IndexFields, in bits
	char indexfield_index[512];	// for IndexFields
	char indexfield_data[512];	// for IndexFields
	uint8_t indexfield_flags;	// for IndexFields
	uint8_t indexfield_size;	// for IndexFields

	lock_t mutex;			// for Mutex

	uint8_t cpu_id;			// for Processor

	char buffer[512];		// for Buffer field
	uint64_t buffer_offset;		// for Buffer field, in bits
	uint64_t buffer_size;		// for Buffer field, in bits
} acpi_handle_t;

acpi_fadt_t *fadt;
acpi_aml_t *dsdt;
acpi_handle_t *acpi_namespace;

// ACPI table functions
void acpi_init();
int acpi_checksum(void *);
void *acpi_scan(char *, size_t);
void acpi_load_fadt();

// ACPI namespace functions
void acpi_create_namespace();
int acpins_is_name(char);
size_t acpins_eval_integer(uint8_t *, uint64_t *);
size_t acpins_parse_pkgsize(uint8_t *, size_t *);
void acpins_register_scope(uint8_t *, size_t);
size_t acpins_create_scope(void *);
size_t acpins_create_opregion(void *);
size_t acpins_create_field(void *);
size_t acpins_create_method(void *);
size_t acpins_create_device(void *);
size_t acpins_create_name(void *);
size_t acpins_create_alias(void *);
size_t acpins_create_mutex(void *);
size_t acpins_create_indexfield(void *);
size_t acpins_create_package(acpi_object_t *, void *);
size_t acpins_create_processor(void *);
size_t acpins_create_wordfield(void *);
acpi_handle_t *acpins_resolve(char *);




