
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#define ZERO_OP				0x00
#define ONE_OP				0x01
#define ALIAS_OP			0x06
#define NAME_OP				0x08
#define BYTEPREFIX			0x0A
#define WORDPREFIX			0x0B
#define DWORDPREFIX			0x0C
#define STRINGPREFIX			0x0D
#define QWORDPREFIX			0x0E
#define SCOPE_OP			0x10
#define BUFFER_OP			0x11
#define PACKAGE_OP			0x12
#define VAR_PACKAGE_OP			0x13
#define METHOD_OP			0x14
#define DUAL_PREFIX			0x2E
#define MULTI_PREFIX			0x2F
#define EXTOP_PREFIX			0x5B
#define ROOT_CHAR			0x5C
#define PARENT_CHAR			0x5E
#define LOCAL1_OP			0x61
#define LOCAL2_OP			0x62
#define LOCAL3_OP			0x63
#define LOCAL4_OP			0x64
#define LOCAL5_OP			0x65
#define LOCAL6_OP			0x66
#define LOCAL7_OP			0x67
#define ARG0_OP				0x68
#define ARG1_OP				0x69
#define ARG2_OP				0x6A
#define ARG3_OP				0x6B
#define ARG4_OP				0x6C
#define ARG5_OP				0x6D
#define ARG6_OP				0x6E
#define STORE_OP			0x70
#define ONES_OP				0xFF

// Extended opcodes
#define OPREGION			0x80
#define FIELD				0x81
#define DEVICE				0x82
#define PROCESSOR			0x83

// OpRegion
#define OPREGION_MEMORY			0x00
#define OPREGION_IO			0x01
#define OPREGION_PCI			0x02
#define OPREGION_EC			0x03
#define OPREGION_SMBUS			0x04
#define OPREGION_CMOS			0x05
#define OPREGION_OEM			0x80

// Fields
#define FIELD_ANY_ACCESS		0x00
#define FIELD_BYTE_ACCESS		0x01
#define FIELD_WORD_ACCESS		0x02
#define FIELD_DWORD_ACCESS		0x03
#define FIELD_QWORD_ACCESS		0x04
#define FIELD_LOCK			0x10
#define FIELD_PRESERVE			0x00
#define FIELD_WRITE_ONES		0x01
#define FIELD_WRITE_ZEROES		0x02

// Methods
#define METHOD_ARGC_MASK		0x07
#define METHOD_SERIALIZED		0x08




