
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <kprintf.h>
#include <mm.h>
#include <string.h>
#include <aml_opcodes.h>

#define CODE_WINDOW		32768

uint8_t *acpi_aml_code;
size_t acpi_aml_allocation = 0;
size_t acpi_aml_size = 0;
size_t acpi_aml_count = 0;
extern char aml_test[];
char aml_path[512];

acpi_handle_t *acpi_namespace;
size_t acpi_namespace_entries = 0;

void aml_load_table(void *);

// acpi_create_namespace(): Initializes the AML interpreter and creates the ACPI namespace
// Param:	Nothing
// Return:	Nothing

void acpi_create_namespace()
{
	memset(aml_path, 0, 512);
	aml_path[0] = ROOT_CHAR;

	acpi_aml_code = kmalloc(CODE_WINDOW);
	acpi_aml_allocation = CODE_WINDOW;
	acpi_namespace = kcalloc(sizeof(acpi_handle_t), ACPI_MAX_NAMESPACE_ENTRIES);

	// load the DSDT
	aml_load_table(dsdt);

	// load all SSDTs
	size_t index = 0;
	acpi_aml_t *ssdt = acpi_scan("SSDT", index);
	while(ssdt != NULL)
	{
		aml_load_table(ssdt);
		index++;
		ssdt = acpi_scan("SSDT", index);
	}

	// the PSDT is treated the same way as the SSDT
	// scan for PSDTs too for compatibility with some ACPI 1.0 PCs
	index = 0;
	acpi_aml_t *psdt = acpi_scan("PSDT", index);
	while(psdt != NULL)
	{
		aml_load_table(psdt);
		index++;
		psdt = acpi_scan("PSDT", index);
	}

	//aml_load_table(aml_test);	// custom AML table just for testing

	// create the namespace with all the objects
	// most of the functions are recursive
	aml_register_scope(acpi_aml_code, acpi_aml_size);
	while(1);
}

// aml_load_table(): Loads an AML table
// Param:	void *ptr - pointer to table
// Return:	Nothing

void aml_load_table(void *ptr)
{
	acpi_aml_t *table = (acpi_aml_t*)ptr;
	while(acpi_aml_size + table->header.length >= acpi_aml_allocation)
	{
		acpi_aml_allocation += CODE_WINDOW;
		acpi_aml_code = krealloc(acpi_aml_code, acpi_aml_allocation);
	}

	// memcpy the actual AML code
	memcpy(acpi_aml_code + acpi_aml_size, table->data, table->header.length - sizeof(acpi_header_t));
	acpi_aml_size += (table->header.length - sizeof(acpi_header_t));

	kprintf("acpi: loaded AML table '%c%c%c%c', total %d bytes of AML code.\n", table->header.signature[0], table->header.signature[1], table->header.signature[2], table->header.signature[3], acpi_aml_size);

	acpi_aml_count++;
}

// aml_register_scope(): Registers a scope
// Param:	uint8_t *data - data
// Param:	size_t size - size of scope in bytes
// Return:	Nothing

void aml_register_scope(uint8_t *data, size_t size)
{
	size_t count = 0;
	while(count < size)
	{
		switch(data[count])
		{
		/*case ZERO_OP:
		case ONE_OP:
		case ONES_OP:
			count++;
			break;

		case BYTEPREFIX:
			count += 2;
			break;
		case WORDPREFIX:
			count += 3;
			break;
		case DWORDPREFIX:
			count += 5;
			break;
		case QWORDPREFIX:
			count += 9;
			break;
		case STRINGPREFIX:
			count += strlen((const char *)&data[count]) + 1;
			break;*/

		case SCOPE_OP:
			count += aml_create_scope(&data[count]);
			break;

		case METHOD_OP:
			count += aml_create_method(&data[count]);
			break;

		case EXTOP_PREFIX:
			switch(data[count+1])
			{
			case OPREGION:
				count += aml_create_opregion(&data[count]);
				break;
			case FIELD:
				count += aml_create_field(&data[count]);
				break;
			case DEVICE:
				count += aml_create_device(&data[count]);
				break;

			default:
				kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", data[count], data[count+1], data[count+2], data[count+3]);
				while(1);
			}
			break;

		default:
			kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", data[count], data[count+1], data[count+2], data[count+3]);
			while(1);
		}
	}
}

// aml_create_scope(): Creates a scope in the namespace
// Param:	void *data - scope data
// Return:	size_t - size of scope in bytes

size_t aml_create_scope(void *data)
{
	uint8_t *scope = (uint8_t*)data;
	size_t size;
	size_t pkgsize;

	pkgsize = aml_parse_pkgsize(scope + 1, &size);

	size_t i = 0;

	// register the scope
	strcpy(acpi_namespace[acpi_namespace_entries].path, aml_path);
	if(strlen(aml_path) != 0)
	{
		i += strlen(aml_path);
		acpi_namespace[acpi_namespace_entries].path[i] = '.';

		i++;
	}

	size_t j = 0;
	scope += pkgsize + 1;

	size_t name_length = 0;

	if(scope[0] == ROOT_CHAR)
	{
		acpi_namespace[acpi_namespace_entries].path[0] = ROOT_CHAR;
		acpi_namespace[acpi_namespace_entries].path[1] = 0;
		name_length = 2;
	} else if(scope[0] == DUAL_PREFIX)
	{
		name_length = 9;
		scope++;

do_dual:
		j = 0;
		while(aml_is_name(scope[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = scope[j];
			i++;
			j++;
		}

		if(aml_is_name(scope[j]) != 1)
			goto copied_name;

		j = 0;
		scope += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_dual;
	} else if(scope[0] == MULTI_PREFIX)
	{
		scope += 2;

do_multi:
		j = 0;
		while(aml_is_name(scope[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = scope[j];
			i++;
			j++;
			name_length++;
		}

		if(aml_is_name(scope[j]) != 1)
			goto copied_name;

		j = 0;
		scope += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_multi;
	} else
	{
		while(j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = scope[j];
			j++;
			i++;
		}

		name_length = 4;
	}

copied_name:
	kprintf("acpi: found scope %s, size %d bytes\n", acpi_namespace[acpi_namespace_entries].path, size);

	// store the new current path
	char current_path[512];
	strcpy(current_path, aml_path);

	// and update the path
	strcpy(aml_path, acpi_namespace[acpi_namespace_entries].path);

	// put the scope in the namespace
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_SCOPE;
	acpi_namespace[acpi_namespace_entries].size = size - pkgsize - name_length;
	acpi_namespace[acpi_namespace_entries].pointer = (void*)(data + 1 + pkgsize + name_length);

	acpi_namespace_entries++;

	// register the child objects of the scope
	aml_register_scope((uint8_t*)data + 1 + pkgsize + name_length, size - pkgsize - name_length);

	// finally restore the original path
	strcpy(aml_path, current_path);
	return size + 1;
}

// aml_create_opregion(): Creates an OpRegion
// Param:	void *data - OpRegion data
// Return:	size_t - total size of OpRegion in bytes

size_t aml_create_opregion(void *data)
{
	uint8_t *opregion = (uint8_t*)data;
	opregion += 2;		// skip EXTOP_PREFIX and OPREGION opcodes

	size_t i = 0;

	// create a namespace object for the opregion
	strcpy(acpi_namespace[acpi_namespace_entries].path, aml_path);
	if(strlen(aml_path) != 0)
	{
		i += strlen(aml_path);
		acpi_namespace[acpi_namespace_entries].path[i] = '.';

		i++;
	}

	size_t j = 0;
	size_t name_length = 0;

	if(opregion[0] == DUAL_PREFIX)
	{
		name_length = 9;
		opregion++;

do_dual:
		j = 0;
		while(aml_is_name(opregion[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = opregion[j];
			i++;
			j++;
		}

		if(aml_is_name(opregion[j]) != 1)
			goto copied_name;

		j = 0;
		opregion += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_dual;
	} else if(opregion[0] == MULTI_PREFIX)
	{
		opregion += 2;		// skip MULTI_PREFIX and count byte

do_multi:
		j = 0;
		while(aml_is_name(opregion[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = opregion[j];
			i++;
			j++;
			name_length++;
		}

		if(aml_is_name(opregion[j]) != 1)
			goto copied_name;

		j = 0;
		opregion += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_multi;
	} else
	{
		while(j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = opregion[j];
			j++;
			i++;
		}

		name_length = 4;
	}

copied_name:
	opregion = (uint8_t*)data;

	size_t size = name_length + 2;
	uint64_t integer;
	size_t integer_size;

	acpi_namespace[acpi_namespace_entries].op_address_space = opregion[size];
	size++;

	integer_size = aml_eval_integer(&opregion[size], &integer);
	if(integer_size == 0)
	{
		kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", opregion[size], opregion[size+1], opregion[size+2], opregion[size+3]);
		while(1);
	}

	acpi_namespace[acpi_namespace_entries].op_base = integer;
	size += integer_size;

	integer_size = aml_eval_integer(&opregion[size], &integer);
	if(integer_size == 0)
	{
		kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", opregion[size], opregion[size+1], opregion[size+2], opregion[size+3]);
		while(1);
	}

	acpi_namespace[acpi_namespace_entries].op_length = integer;
	size += integer_size;

	kprintf("acpi: found OpRegion %s: ", acpi_namespace[acpi_namespace_entries].path);
	switch(acpi_namespace[acpi_namespace_entries].op_address_space)
	{
	case OPREGION_MEMORY:
		kprintf("MMIO: 0x%xq-0x%xq\n", acpi_namespace[acpi_namespace_entries].op_base, acpi_namespace[acpi_namespace_entries].op_base + acpi_namespace[acpi_namespace_entries].op_length);
		break;
	case OPREGION_IO:
		kprintf("I/O port: 0x%xw-0x%xw\n", (uint16_t)(acpi_namespace[acpi_namespace_entries].op_base), (uint16_t)(acpi_namespace[acpi_namespace_entries].op_base + acpi_namespace[acpi_namespace_entries].op_length));
		break;
	case OPREGION_PCI:
		kprintf("PCI config: 0x%xw-0x%xw\n", (uint16_t)(acpi_namespace[acpi_namespace_entries].op_base), (uint16_t)(acpi_namespace[acpi_namespace_entries].op_base + acpi_namespace[acpi_namespace_entries].op_length));
		break;

	// TO-DO: I should read about the embedded controller and implement more
	// possible address spaces here.

	default:
		kprintf("unsupported address space ID 0x%xb\n", acpi_namespace[acpi_namespace_entries].op_address_space);
		while(1);
	}

	acpi_namespace_entries++;
	return size;
}

// aml_create_field(): Creates a Field object in the namespace
// Param:	void *data - pointer to field data
// Return:	size_t - total size of field in bytes

size_t aml_create_field(void *data)
{
	uint8_t *field = (uint8_t*)data;
	field += 2;		// skip opcode

	// package size
	size_t pkgsize, size;

	pkgsize = aml_parse_pkgsize(field, &size);
	field += pkgsize;

	// determine name of opregion
	acpi_handle_t *opregion;
	char opregion_name[512];
	size_t name_size = 0;

	if(field[0] == DUAL_PREFIX)
	{
		name_size = 9;
		field++;
		memcpy(opregion_name, field, 4);
		opregion_name[4] = '.';
		memcpy(opregion_name + 5, field + 4, 4);
	} else if(field[0] == MULTI_PREFIX)
	{
		field += 2;		// skip MULTI_PREFIX and byte count data
		name_size = 2;

		size_t i = 0, j = 0;

do_multi:
		while(i < 4)
		{
			name_size += 4;
			opregion_name[j] = field[i];
			i++;
			j++;
		}

		if(aml_is_name(field[i]) == 1)
		{
			opregion_name[j] = '.';
			j++;
			i = 0;
			field += 4;
			goto do_multi;
		}
	} else
	{
		// normal four-char name here
		memcpy(opregion_name, field, 4);
		name_size = 4;
	}

	opregion = aml_resolve(opregion_name);
	if(!opregion)
	{
		kprintf("acpi: error parsing field for non-existant OpRegion %s, ignoring...\n", opregion_name);
		return size + 2;
	}

	// parse the field's entries now
	uint8_t field_flags;
	field = (uint8_t*)data + 2 + pkgsize + name_size;
	field_flags = field[0];

	kprintf("acpi: found field for OpRegion %s, flags 0x%xb (", opregion->path, field_flags);
	switch(field_flags & 0x0F)
	{
	case FIELD_ANY_ACCESS:
		kprintf("any ");
		break;
	case FIELD_BYTE_ACCESS:
		kprintf("byte ");
		break;
	case FIELD_WORD_ACCESS:
		kprintf("word ");
		break;
	case FIELD_DWORD_ACCESS:
		kprintf("dword ");
		break;
	case FIELD_QWORD_ACCESS:
		kprintf("qword ");
		break;
	default:
		kprintf("undefined access size: assuming any, ");
		break;
	}

	if(field_flags & FIELD_LOCK)
		kprintf("lock ");

	switch((field_flags >> 5) & 0x0F)
	{
	case FIELD_PRESERVE:
		kprintf("preserve");
		break;
	case FIELD_WRITE_ONES:
		kprintf("write ones");
		break;
	case FIELD_WRITE_ZEROES:
		kprintf("write zeroes");
		break;
	default:
		kprintf("undefined update type");
		break;
	}

	kprintf(")\n");

	acpi_namespace_entries++;

	field++;		// actual field objects
	size_t byte_count = (size_t)((size_t)field - (size_t)data);

	uint64_t current_offset = 0;

	while(byte_count < size)
	{
		kprintf("acpi: field %c%c%c%c: size %d bits, at bit offset %d\n", field[0], field[1], field[2], field[3], field[4]);
		acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_FIELD;
		memcpy(acpi_namespace[acpi_namespace_entries].path, aml_path, strlen(aml_path));
		acpi_namespace[acpi_namespace_entries].path[strlen(aml_path)] = '.';
		memcpy(acpi_namespace[acpi_namespace_entries].path + strlen(aml_path) + 1, field, 4);
		strcpy(acpi_namespace[acpi_namespace_entries].field_opregion, opregion->path);
		acpi_namespace[acpi_namespace_entries].field_flags = field_flags;
		acpi_namespace[acpi_namespace_entries].field_size = field[4];
		acpi_namespace[acpi_namespace_entries].field_offset = current_offset;

		current_offset += (uint64_t)(field[4]);
		acpi_namespace_entries++;

		field += 5;
		byte_count += 5;
	}

	return size + 2;
}

// aml_create_method(): Registers a control method in the namespace
// Param:	void *data - pointer to AML code
// Return:	size_t - total size in bytes for skipping

size_t aml_create_method(void *data)
{
	uint8_t *method = (uint8_t*)data;
	method++;		// skip over METHOD_OP

	size_t size, pkgsize;
	pkgsize = aml_parse_pkgsize(method, &size);
	method += pkgsize;

	size_t i = 0;

	// create a namespace object for the method
	strcpy(acpi_namespace[acpi_namespace_entries].path, aml_path);
	if(strlen(aml_path) != 0)
	{
		i += strlen(aml_path);
		acpi_namespace[acpi_namespace_entries].path[i] = '.';

		i++;
	}

	size_t j = 0;
	size_t name_length = 0;

	if(method[0] == DUAL_PREFIX)
	{
		name_length = 9;
		method++;

do_dual:
		j = 0;
		while(aml_is_name(method[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = method[j];
			i++;
			j++;
		}

		if(aml_is_name(method[j]) != 1)
			goto copied_name;

		j = 0;
		method += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_dual;
	} else if(method[0] == MULTI_PREFIX)
	{
		method += 2;		// skip MULTI_PREFIX and count byte

do_multi:
		j = 0;
		while(aml_is_name(method[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = method[j];
			i++;
			j++;
			name_length++;
		}

		if(aml_is_name(method[j]) != 1)
			goto copied_name;

		j = 0;
		method += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_multi;
	} else
	{
		while(j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = method[j];
			j++;
			i++;
		}

		name_length = 4;
	}

copied_name:

	// get the method's flags
	method = (uint8_t*)data;
	method += pkgsize + name_length + 1;

	// put the method in the namespace
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_METHOD;
	acpi_namespace[acpi_namespace_entries].method_flags = method[0];
	acpi_namespace[acpi_namespace_entries].pointer = (void*)(method);
	acpi_namespace[acpi_namespace_entries].size = size - pkgsize - name_length - 1;

	kprintf("acpi: control method %s, flags 0x%xb (argc %d ", acpi_namespace[acpi_namespace_entries].path, method[0], method[0] & METHOD_ARGC_MASK);
	if(method[0] & METHOD_SERIALIZED)
		kprintf("serialized");
	else
		kprintf("non-serialized");

	kprintf(")\n");
	return size + 1;
}

// aml_create_device(): Creates a device scope in the namespace
// Param:	void *data - device scope data
// Return:	size_t - size of device scope in bytes

size_t aml_create_device(void *data)
{
	uint8_t *device = (uint8_t*)data;
	size_t size;
	size_t pkgsize;

	pkgsize = aml_parse_pkgsize(device + 2, &size);

	size_t i = 0;

	// register the device
	strcpy(acpi_namespace[acpi_namespace_entries].path, aml_path);
	if(strlen(aml_path) != 0)
	{
		i += strlen(aml_path);
		acpi_namespace[acpi_namespace_entries].path[i] = '.';

		i++;
	}

	size_t j = 0;
	device += pkgsize + 2;

	size_t name_length = 0;

	if(device[0] == DUAL_PREFIX)
	{
		name_length = 9;
		device++;

do_dual:
		j = 0;
		while(aml_is_name(device[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = device[j];
			i++;
			j++;
		}

		if(aml_is_name(device[j]) != 1)
			goto copied_name;

		j = 0;
		device += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_dual;
	} else if(device[0] == MULTI_PREFIX)
	{
		device += 2;

do_multi:
		j = 0;
		while(aml_is_name(device[j]) == 1 && j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = device[j];
			i++;
			j++;
			name_length++;
		}

		if(aml_is_name(device[j]) != 1)
			goto copied_name;

		j = 0;
		device += 4;
		acpi_namespace[acpi_namespace_entries].path[i] = '.';
		i++;
		goto do_multi;
	} else
	{
		while(j < 4)
		{
			acpi_namespace[acpi_namespace_entries].path[i] = device[j];
			j++;
			i++;
		}

		name_length = 4;
	}

copied_name:
	kprintf("acpi: found device scope %s, size %d bytes\n", acpi_namespace[acpi_namespace_entries].path, size);

	// store the new current path
	char current_path[512];
	strcpy(current_path, aml_path);

	// and update the path
	strcpy(aml_path, acpi_namespace[acpi_namespace_entries].path);

	// put the scope in the namespace
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_SCOPE;
	acpi_namespace[acpi_namespace_entries].size = size - pkgsize - name_length;
	acpi_namespace[acpi_namespace_entries].pointer = (void*)(data + 2 + pkgsize + name_length);

	acpi_namespace_entries++;

	// register the child objects of the device scope
	aml_register_scope((uint8_t*)data + 2 + pkgsize + name_length, size - pkgsize - name_length);

	// finally restore the original path
	strcpy(aml_path, current_path);
	return size + 2;
}

// aml_resolve(): Returns a namespace object from its path
// Param:	char *path - 4-char object name or full path
// Return:	acpi_handle_t * - pointer to namespace object, NULL on error

acpi_handle_t *aml_resolve(char *path)
{
	size_t i = 0;

	if(path[0] == ROOT_CHAR)		// full path?
	{
		// yep, search for the absolute path
		while(i < acpi_namespace_entries)
		{
			if(strcmp(acpi_namespace[i].path, path) == 0)
				return &acpi_namespace[i];

			else
				i++;
		}
	} else			// 4-char name here
	{
		while(i < acpi_namespace_entries)
		{
			if(memcmp(acpi_namespace[i].path + strlen(acpi_namespace[i].path) - 4, path, 4) == 0)
				return &acpi_namespace[i];

			else
				i++;
		}
	}

	kprintf("acpi: namespace object %s doesn't exist.\n");
	return NULL;
}



