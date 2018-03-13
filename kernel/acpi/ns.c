
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <acpi.h>
#include <kprintf.h>
#include <mm.h>
#include <string.h>
#include <aml_opcodes.h>

#define CODE_WINDOW		65536

uint8_t *acpi_acpins_code;
size_t acpi_acpins_allocation = 0;
size_t acpi_acpins_size = 0;
size_t acpi_acpins_count = 0;
extern char acpins_test[];
char acpins_path[512];

acpi_handle_t *acpi_namespace;
size_t acpi_namespace_entries = 0;

void acpins_load_table(void *);

// acpins_resolve_path(): Resolves a path
// Param:	char *fullpath - destination
// Param:	uint8_t *path - path to resolve
// Return:	size_t - size of path data parsed in AML

size_t acpins_resolve_path(char *fullpath, uint8_t *path)
{
	size_t name_size = 0;
	size_t multi_count = 0;
	size_t current_count = 0;

	memset(fullpath, 0, 512);

	if(path[0] == ROOT_CHAR)
	{
		name_size = 1;
		fullpath[0] = ROOT_CHAR;
		fullpath[1] = 0;
		path++;
		if(acpins_is_name(path[0]) || path[0] == DUAL_PREFIX || path[0] == MULTI_PREFIX)
		{
			fullpath[1] = '.';
			fullpath[2] = 0;
			goto start;
		} else
			return name_size;
	}

	strcpy(fullpath, acpins_path);
	fullpath[strlen(fullpath)] = '.';

start:
	while(path[0] == PARENT_CHAR)
	{
		path++;
		if(strlen(fullpath) <= 2)
			break;

		name_size++;
		fullpath[strlen(fullpath) - 5] = 0;
		memset(fullpath + strlen(fullpath), 0, 32);
	}

	if(path[0] == DUAL_PREFIX)
	{
		name_size += 9;
		path++;
		memcpy(fullpath + strlen(fullpath), path, 4);
		fullpath[strlen(fullpath)] = '.';
		memcpy(fullpath + strlen(fullpath), path + 4, 4);
	} else if(path[0] == MULTI_PREFIX)
	{
		// skip MULTI_PREFIX and name count
		name_size += 2;
		path++;

		// get name count here
		multi_count = (size_t)path[0];
		path++;

		current_count = 0;
		while(current_count < multi_count)
		{
			name_size += 4;
			memcpy(fullpath + strlen(fullpath), path, 4);
			path += 4;
			current_count++;
			if(current_count >= multi_count)
				break;

			fullpath[strlen(fullpath)] = '.';
		}
	} else
	{
		name_size += 4;
		memcpy(fullpath + strlen(fullpath), path, 4);
	}

	return name_size;
}

// acpins_increment_namespace(): Increments the namespace counter
// Param:	Nothing
// Return:	Nothing

void acpins_increment_namespace()
{
	acpi_namespace_entries++;
	if((acpi_namespace_entries % ACPI_MAX_NAMESPACE_ENTRIES) == 0)
		acpi_namespace = krealloc(acpi_namespace, (acpi_namespace_entries + ACPI_MAX_NAMESPACE_ENTRIES + 1) * sizeof(acpi_handle_t));
}

// acpi_create_namespace(): Initializes the AML interpreter and creates the ACPI namespace
// Param:	Nothing
// Return:	Nothing

void acpi_create_namespace()
{
	memset(acpins_path, 0, 512);
	acpins_path[0] = ROOT_CHAR;

	acpi_acpins_code = kmalloc(CODE_WINDOW);
	acpi_acpins_allocation = CODE_WINDOW;
	acpi_namespace = kcalloc(sizeof(acpi_handle_t), ACPI_MAX_NAMESPACE_ENTRIES);

	//acpins_load_table(acpins_test);	// custom AML table just for testing

	// load the DSDT
	acpins_load_table(dsdt);

	// load all SSDTs
	size_t index = 0;
	acpi_aml_t *ssdt = acpi_scan("SSDT", index);
	while(ssdt != NULL)
	{
		acpins_load_table(ssdt);
		index++;
		ssdt = acpi_scan("SSDT", index);
	}

	// the PSDT is treated the same way as the SSDT
	// scan for PSDTs too for compatibility with some ACPI 1.0 PCs
	index = 0;
	acpi_aml_t *psdt = acpi_scan("PSDT", index);
	while(psdt != NULL)
	{
		acpins_load_table(psdt);
		index++;
		psdt = acpi_scan("PSDT", index);
	}

	// create the namespace with all the objects
	// most of the functions are recursive
	acpins_register_scope(acpi_acpins_code, acpi_acpins_size);

	kprintf("acpi: ACPI namespace created, total of %d predefined objects.\n", acpi_namespace_entries);
}

// acpins_load_table(): Loads an AML table
// Param:	void *ptr - pointer to table
// Return:	Nothing

void acpins_load_table(void *ptr)
{
	acpi_aml_t *table = (acpi_aml_t*)ptr;
	while(acpi_acpins_size + table->header.length >= acpi_acpins_allocation)
	{
		acpi_acpins_allocation += CODE_WINDOW;
		acpi_acpins_code = krealloc(acpi_acpins_code, acpi_acpins_allocation);
	}

	// memcpy the actual AML code
	memmove(acpi_acpins_code + acpi_acpins_size, table->data, table->header.length - sizeof(acpi_header_t));
	acpi_acpins_size += (table->header.length - sizeof(acpi_header_t));

	kprintf("acpi: loaded AML table '%c%c%c%c', total %d bytes of AML code.\n", table->header.signature[0], table->header.signature[1], table->header.signature[2], table->header.signature[3], acpi_acpins_size);

	acpi_acpins_count++;
}

// acpins_register_scope(): Registers a scope
// Param:	uint8_t *data - data
// Param:	size_t size - size of scope in bytes
// Return:	Nothing

void acpins_register_scope(uint8_t *data, size_t size)
{
	size_t count = 0;
	size_t pkgsize;
	while(count < size)
	{
		switch(data[count])
		{
		case ZERO_OP:
		case ONE_OP:
		case ONES_OP:
		case NOP_OP:
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
			break;

		case NAME_OP:
			count += acpins_create_name(&data[count]);
			break;

		case ALIAS_OP:
			count += acpins_create_alias(&data[count]);
			break;

		case SCOPE_OP:
			count += acpins_create_scope(&data[count]);
			break;

		case METHOD_OP:
			count += acpins_create_method(&data[count]);
			break;

		case BUFFER_OP:
		case PACKAGE_OP:
		case VARPACKAGE_OP:
			count++;
			acpins_parse_pkgsize(&data[count], &pkgsize);
			count += pkgsize;
			break;

		case WORDFIELD_OP:
			count += acpins_create_wordfield(&data[count]);
			break;

		case EXTOP_PREFIX:
			switch(data[count+1])
			{
			case MUTEX:
				count += acpins_create_mutex(&data[count]);
				break;
			case OPREGION:
				count += acpins_create_opregion(&data[count]);
				break;
			case FIELD:
				count += acpins_create_field(&data[count]);
				break;
			case DEVICE:
				count += acpins_create_device(&data[count]);
				break;
			case INDEXFIELD:
				count += acpins_create_indexfield(&data[count]);
				break;
			case PROCESSOR:
				count += acpins_create_processor(&data[count]);
				break;

			default:
				kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", data[count], data[count+1], data[count+2], data[count+3]);
				while(1);
			}
			break;

		case IF_OP:
		case ELSE_OP:
			count++;
			acpins_parse_pkgsize(&data[count], &pkgsize);
			count += pkgsize;
			break;

		default:
			kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", data[count], data[count+1], data[count+2], data[count+3]);
			while(1);
		}
	}
}

// acpins_create_scope(): Creates a scope in the namespace
// Param:	void *data - scope data
// Return:	size_t - size of scope in bytes

size_t acpins_create_scope(void *data)
{
	uint8_t *scope = (uint8_t*)data;
	size_t size;
	size_t pkgsize;

	pkgsize = acpins_parse_pkgsize(scope + 1, &size);

	// register the scope
	scope += pkgsize + 1;
	size_t name_length = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, scope);

	kprintf("acpi: scope %s, size %d bytes\n", acpi_namespace[acpi_namespace_entries].path, size);

	// store the new current path
	char current_path[512];
	strcpy(current_path, acpins_path);

	// and update the path
	strcpy(acpins_path, acpi_namespace[acpi_namespace_entries].path);

	// put the scope in the namespace
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_SCOPE;
	acpi_namespace[acpi_namespace_entries].size = size - pkgsize - name_length;
	acpi_namespace[acpi_namespace_entries].pointer = (void*)(data + 1 + pkgsize + name_length);

	acpins_increment_namespace();

	// register the child objects of the scope
	acpins_register_scope((uint8_t*)data + 1 + pkgsize + name_length, size - pkgsize - name_length);

	// finally restore the original path
	strcpy(acpins_path, current_path);
	return size + 1;
}

// acpins_create_opregion(): Creates an OpRegion
// Param:	void *data - OpRegion data
// Return:	size_t - total size of OpRegion in bytes

size_t acpins_create_opregion(void *data)
{
	uint8_t *opregion = (uint8_t*)data;
	opregion += 2;		// skip EXTOP_PREFIX and OPREGION opcodes

	// create a namespace object for the opregion
	size_t name_length = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, opregion);

	opregion = (uint8_t*)data;

	size_t size = name_length + 2;
	uint64_t integer;
	size_t integer_size;

	acpi_namespace[acpi_namespace_entries].op_address_space = opregion[size];
	size++;

	integer_size = acpins_eval_integer(&opregion[size], &integer);
	if(integer_size == 0)
	{
		kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", opregion[size], opregion[size+1], opregion[size+2], opregion[size+3]);
		while(1);
	}

	acpi_namespace[acpi_namespace_entries].op_base = integer;
	size += integer_size;

	integer_size = acpins_eval_integer(&opregion[size], &integer);
	if(integer_size == 0)
	{
		kprintf("acpi: undefined opcode, sequence: %xb %xb %xb %xb\n", opregion[size], opregion[size+1], opregion[size+2], opregion[size+3]);
		while(1);
	}

	acpi_namespace[acpi_namespace_entries].op_length = integer;
	size += integer_size;

	kprintf("acpi: OpRegion %s: ", acpi_namespace[acpi_namespace_entries].path);
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

	acpins_increment_namespace();
	return size;
}

// acpins_create_field(): Creates a Field object in the namespace
// Param:	void *data - pointer to field data
// Return:	size_t - total size of field in bytes

size_t acpins_create_field(void *data)
{
	uint8_t *field = (uint8_t*)data;
	field += 2;		// skip opcode

	// package size
	size_t pkgsize, size;

	pkgsize = acpins_parse_pkgsize(field, &size);
	field += pkgsize;

	// determine name of opregion
	acpi_handle_t *opregion;
	char opregion_name[512];
	size_t name_size = 0;

	name_size = acpins_resolve_path(opregion_name, field);

	opregion = acpins_resolve(opregion_name);
	if(!opregion)
	{
		kprintf("acpi: error parsing field for non-existant OpRegion %s, ignoring...\n", opregion_name);
		return size + 2;
	}

	// parse the field's entries now
	uint8_t field_flags;
	field = (uint8_t*)data + 2 + pkgsize + name_size;
	field_flags = field[0];

	kprintf("acpi: field for OpRegion %s, flags 0x%xb (", opregion->path, field_flags);
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
		kprintf("ones");
		break;
	case FIELD_WRITE_ZEROES:
		kprintf("zeroes");
		break;
	default:
		kprintf("undefined update type");
		break;
	}

	kprintf(")\n");

	acpins_increment_namespace();

	field++;		// actual field objects
	size_t byte_count = (size_t)((size_t)field - (size_t)data);

	uint64_t current_offset = 0;
	size_t skip_size, skip_bits;

	while(byte_count < size)
	{
		while(field[0] == 0)		// skipping?
		{
			field++;
			byte_count++;

			skip_size = acpins_parse_pkgsize(field, &skip_bits);
			current_offset += skip_bits;

			field += skip_size;
			byte_count += skip_size;
		}

		kprintf("acpi: field %c%c%c%c: size %d bits, at bit offset %d\n", field[0], field[1], field[2], field[3], field[4], current_offset);
		acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_FIELD;
		memcpy(acpi_namespace[acpi_namespace_entries].path, acpins_path, strlen(acpins_path));
		acpi_namespace[acpi_namespace_entries].path[strlen(acpins_path)] = '.';
		memcpy(acpi_namespace[acpi_namespace_entries].path + strlen(acpins_path) + 1, field, 4);
		strcpy(acpi_namespace[acpi_namespace_entries].field_opregion, opregion->path);
		acpi_namespace[acpi_namespace_entries].field_flags = field_flags;
		acpi_namespace[acpi_namespace_entries].field_size = field[4];
		acpi_namespace[acpi_namespace_entries].field_offset = current_offset;

		current_offset += (uint64_t)(field[4]);
		acpins_increment_namespace();

		field += 5;
		byte_count += 5;
	}

	return size + 2;
}

// acpins_create_method(): Registers a control method in the namespace
// Param:	void *data - pointer to AML code
// Return:	size_t - total size in bytes for skipping

size_t acpins_create_method(void *data)
{
	uint8_t *method = (uint8_t*)data;
	method++;		// skip over METHOD_OP

	size_t size, pkgsize;
	pkgsize = acpins_parse_pkgsize(method, &size);
	method += pkgsize;

	// create a namespace object for the method
	size_t name_length = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, method);

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

// acpins_create_device(): Creates a device scope in the namespace
// Param:	void *data - device scope data
// Return:	size_t - size of device scope in bytes

size_t acpins_create_device(void *data)
{
	uint8_t *device = (uint8_t*)data;
	size_t size;
	size_t pkgsize;

	pkgsize = acpins_parse_pkgsize(device + 2, &size);

	// register the device
	device += pkgsize + 2;

	size_t name_length = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, device);

	kprintf("acpi: device scope %s, size %d bytes\n", acpi_namespace[acpi_namespace_entries].path, size);

	// store the new current path
	char current_path[512];
	strcpy(current_path, acpins_path);

	// and update the path
	strcpy(acpins_path, acpi_namespace[acpi_namespace_entries].path);

	// put the device scope in the namespace
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_DEVICE;
	acpi_namespace[acpi_namespace_entries].size = size - pkgsize - name_length;
	acpi_namespace[acpi_namespace_entries].pointer = (void*)(data + 2 + pkgsize + name_length);

	acpins_increment_namespace();

	// register the child objects of the device scope
	acpins_register_scope((uint8_t*)data + 2 + pkgsize + name_length, size - pkgsize - name_length);

	// finally restore the original path
	strcpy(acpins_path, current_path);
	return size + 2;
}

// acpins_create_name(): Creates a name in the namespace
// Param:	void *data - pointer to data
// Return:	size_t - total size in bytes, for skipping

size_t acpins_create_name(void *data)
{
	uint8_t *name = (uint8_t*)data;
	name++;			// skip NAME_OP

	// create a namespace object for the name object
	size_t name_length = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, name);

	name += name_length;
	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_NAME;

	size_t return_size = name_length + 1;

	if(name[0] == PACKAGE_OP)
	{
		acpi_namespace[acpi_namespace_entries].object.type = ACPI_PACKAGE;
		acpi_namespace[acpi_namespace_entries].object.package = kcalloc(sizeof(acpi_object_t), ACPI_MAX_PACKAGE_ENTRIES);
		acpi_namespace[acpi_namespace_entries].object.package_size = acpins_create_package(acpi_namespace[acpi_namespace_entries].object.package, &name[0]);

		kprintf("acpi: package object %s, entry count %d\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].object.package_size);
		acpins_increment_namespace();
		return return_size;
	}

	uint64_t integer;
	size_t integer_size = acpins_eval_integer(name, &integer);

	if(integer_size != 0)
	{
		acpi_namespace[acpi_namespace_entries].object.type = ACPI_INTEGER;
		acpi_namespace[acpi_namespace_entries].object.integer = integer;
	} else if(name[0] == BUFFER_OP)
	{
		acpi_namespace[acpi_namespace_entries].object.type = ACPI_BUFFER;
		acpi_namespace[acpi_namespace_entries].object.buffer = &name[1];
		acpins_parse_pkgsize(&name[1], &acpi_namespace[acpi_namespace_entries].object.buffer_size);
	} else if(name[0] == STRINGPREFIX)
	{
		acpi_namespace[acpi_namespace_entries].object.type = ACPI_STRING;
		acpi_namespace[acpi_namespace_entries].object.string = (char*)&name[1];
	} else
	{
		kprintf("acpi: undefined opcode in Name(), sequence: %xb %xb %xb %xb\n", name[0], name[1], name[2], name[3]);
		while(1);
	}

	if(acpi_namespace[acpi_namespace_entries].object.type == ACPI_INTEGER)
		kprintf("acpi: integer object %s, value 0x%xq\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].object.integer);
	else if(acpi_namespace[acpi_namespace_entries].object.type == ACPI_BUFFER)
		kprintf("acpi: buffer object %s\n", acpi_namespace[acpi_namespace_entries].path);
	else if(acpi_namespace[acpi_namespace_entries].object.type == ACPI_STRING)
		kprintf("acpi: string object %s: '%s'\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].object.string);

	acpins_increment_namespace();
	return return_size;
}

// acpins_create_alias(): Creates an alias object in the namespace
// Param:	void *data - pointer to data
// Return:	size_t - total size in bytes, for skipping

size_t acpins_create_alias(void *data)
{
	size_t return_size = 1;
	uint8_t *alias = (uint8_t*)data;
	alias++;		// skip ALIAS_OP

	size_t name_size;

	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_ALIAS;
	name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].alias, alias);

	return_size += name_size;
	alias += name_size;

	name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, alias);

	kprintf("acpi: alias %s for object %s\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].alias);

	acpins_increment_namespace();
	return_size += name_size;
	return return_size;
}

// acpins_create_mutex(): Creates a Mutex object in the namespace
// Param:	void *data - pointer to data
// Return:	size_t - total size in bytes, for skipping

size_t acpins_create_mutex(void *data)
{
	size_t return_size = 2;
	uint8_t *mutex = (uint8_t*)data;
	mutex += 2;		// skip MUTEX_OP

	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_MUTEX;
	size_t name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, mutex);

	return_size += name_size;
	return_size++;

	kprintf("acpi: mutex object %s\n", acpi_namespace[acpi_namespace_entries].path);

	acpins_increment_namespace();
	return return_size;
}

// acpins_create_indexfield(): Creates an IndexField object in the namespace
// Param:	void *data - pointer to indexfield data
// Return:	size_t - total size of indexfield in bytes

size_t acpins_create_indexfield(void *data)
{
	uint8_t *indexfield = (uint8_t*)data;
	indexfield += 2;		// skip INDEXFIELD_OP

	size_t pkgsize, size;
	pkgsize = acpins_parse_pkgsize(indexfield, &size);

	indexfield += pkgsize;

	// index and data
	char indexr[512], datar[512];
	memset(indexr, 0, 512);
	memset(datar, 0, 512);

	indexfield += acpins_resolve_path(indexr, indexfield);
	indexfield += acpins_resolve_path(datar, indexfield);

	uint8_t flags = indexfield[0];

	kprintf("acpi: IndexField index %s data %s, flags 0x%xb (", indexr, datar, flags);
	switch(flags & 0x0F)
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

	if(flags & FIELD_LOCK)
		kprintf("lock ");

	switch((flags >> 5) & 0x0F)
	{
	case FIELD_PRESERVE:
		kprintf("preserve");
		break;
	case FIELD_WRITE_ONES:
		kprintf("ones");
		break;
	case FIELD_WRITE_ZEROES:
		kprintf("zeroes");
		break;
	default:
		kprintf("undefined update type");
		break;
	}

	kprintf(")\n");

	indexfield++;			// actual field list
	size_t byte_count = (size_t)((size_t)indexfield - (size_t)data);

	uint64_t current_offset = 0;
	size_t skip_size, skip_bits;

	while(byte_count < size)
	{
		while(indexfield[0] == 0)		// skipping?
		{
			indexfield++;
			byte_count++;

			skip_size = acpins_parse_pkgsize(indexfield, &skip_bits);
			current_offset += skip_bits;

			indexfield += skip_size;
			byte_count += skip_size;
		}

		kprintf("acpi: indexfield %c%c%c%c: size %d bits, at bit offset %d\n", indexfield[0], indexfield[1], indexfield[2], indexfield[3], indexfield[4], current_offset);
		acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_INDEXFIELD;
		memcpy(acpi_namespace[acpi_namespace_entries].path, acpins_path, strlen(acpins_path));
		acpi_namespace[acpi_namespace_entries].path[strlen(acpins_path)] = '.';
		memcpy(acpi_namespace[acpi_namespace_entries].path + strlen(acpins_path) + 1, indexfield, 4);

		strcpy(acpi_namespace[acpi_namespace_entries].indexfield_data, datar);
		strcpy(acpi_namespace[acpi_namespace_entries].indexfield_index, indexr);
		acpi_namespace[acpi_namespace_entries].indexfield_flags = flags;
		acpi_namespace[acpi_namespace_entries].indexfield_size = indexfield[4];
		acpi_namespace[acpi_namespace_entries].indexfield_offset = current_offset;

		current_offset += (uint64_t)(indexfield[4]);
		acpins_increment_namespace();

		indexfield += 5;
		byte_count += 5;
	}

	return size + 2;
}

// acpins_create_package(): Creates a package object
// Param:	acpi_object_t *destination - where to create package
// Param:	void *data - package data
// Return:	size_t - size in entries

size_t acpins_create_package(acpi_object_t *destination, void *data)
{
	uint8_t *package = (uint8_t*)data;
	package++;		// skip PACKAGE_OP

	size_t pkgsize, size;
	pkgsize = acpins_parse_pkgsize(package, &size);

	package += pkgsize;
	uint8_t count = package[0];		// entry count
	if(!count)
		return 0;

	// parse actual package contents
	package++;
	uint8_t i = 0;
	size_t j = 0;
	size_t integer_size;
	uint64_t integer;

	//kprintf("acpins_create_package: start:\n");

	while(i < count)
	{
		integer_size = acpins_eval_integer(&package[j], &integer);
		if(integer_size != 0)
		{
			destination[i].type = ACPI_INTEGER;
			destination[i].integer = integer;

			//kprintf("  index %d: integer %d\n", i, integer);
			i++;
			j += integer_size;
		} else if(package[j] == STRINGPREFIX)
		{
			destination[i].type = ACPI_STRING;
			destination[i].string = (char*)&package[j+1];

			//kprintf("  index %d: string %s\n", i, destination[i].string);
			i++;

			j += strlen((char*)&package[j]) + 1;
		} else if(acpins_is_name(package[j]) || package[j] == ROOT_CHAR || package[j] == PARENT_CHAR || package[j] == MULTI_PREFIX || package[j] == DUAL_PREFIX)
		{
			destination[i].type = ACPI_NAME;
			j += acpins_resolve_path(destination[i].name, &package[j]);

			//kprintf("  index %d: name %s\n", i, destination[i].name);
			i++;
		} else if(package[j] == PACKAGE_OP)
		{
			// Package within package!
			destination[i].type = ACPI_PACKAGE;
			destination[i].package = kcalloc(sizeof(acpi_object_t), ACPI_MAX_PACKAGE_ENTRIES);

			//kprintf("  index %d: package\n", i);

			destination[i].package_size = acpins_create_package(destination[i].package, &package[j]);

			j++;
			acpins_parse_pkgsize(&package[j], &size);
			j += size;
			i++;
		} else
		{
			// Undefined here
			kprintf("acpi: undefined opcode in Package(), sequence: %xb %xb %xb %xb\n", package[j], package[j+1], package[j+2], package[j+3]);
			while(1);
		}
	}

	//kprintf("acpins_create_package: end.\n");
	return (size_t)count;
}

// acpins_create_processor(): Creates a Processor object in the namespace
// Param:	void *data - pointer to data
// Return:	size_t - total size in bytes, for skipping

size_t acpins_create_processor(void *data)
{
	uint8_t *processor = (uint8_t*)data;
	processor += 2;			// skip over PROCESSOR_OP

	size_t pkgsize, size;
	pkgsize = acpins_parse_pkgsize(processor, &size);
	processor += pkgsize;

	acpi_namespace[acpi_namespace_entries].type = ACPI_NAMESPACE_PROCESSOR;
	size_t name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, processor);
	processor += name_size;

	acpi_namespace[acpi_namespace_entries].cpu_id = processor[0];

	kprintf("acpi: processor %s ACPI ID %d\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].cpu_id);

	acpins_increment_namespace();

	return size + 2;
}

// acpins_create_wordfield(): Creates a WordField object for a buffer in the namespace
// Param:	void *data - pointer to data
// Return:	size_t - total size in bytes, for skipping

size_t acpins_create_wordfield(void *data)
{
	uint8_t *wordfield = (uint8_t*)data;
	wordfield++;		// skip WORDFIELD_OP

	size_t return_size = 1;

	// buffer name
	size_t name_size;
	name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].buffer, wordfield);

	return_size += name_size;
	wordfield += name_size;

	size_t integer_size;
	uint64_t integer;
	integer_size = acpins_eval_integer(wordfield, &integer);

	acpi_namespace[acpi_namespace_entries].buffer_offset = integer * 8;
	acpi_namespace[acpi_namespace_entries].buffer_size = 16;	// bits

	return_size += integer_size;
	wordfield += name_size;

	name_size = acpins_resolve_path(acpi_namespace[acpi_namespace_entries].path, wordfield);

	kprintf("acpi: field %s for buffer %s, offset %d size %d bits\n", acpi_namespace[acpi_namespace_entries].path, acpi_namespace[acpi_namespace_entries].buffer, acpi_namespace[acpi_namespace_entries].buffer_offset, acpi_namespace[acpi_namespace_entries].buffer_size);

	acpins_increment_namespace();
	return_size += name_size;
	return return_size;
}

// acpins_resolve(): Returns a namespace object from its path
// Param:	char *path - 4-char object name or full path
// Return:	acpi_handle_t * - pointer to namespace object, NULL on error

acpi_handle_t *acpins_resolve(char *path)
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



