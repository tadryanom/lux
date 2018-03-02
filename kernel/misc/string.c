
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <string.h>

// memmove: Moves memory carefully, slow function
// Param:	void *dest - destination
// Param:	const void *src - source
// Param:	size_t size - size of data
// Return:	void * - pointer to destination

void *memmove(void *dest, const void *src, size_t size)
{
	uint8_t *dest2 = dest;
	const uint8_t *src2 = src;
	size_t i = 0;

	while(i < size)
	{
		dest2[i] = src2[i];	// copy byte by byte to be careful
		i++;
	}

	return dest;
}

// strlen(): Returns length of string
// Param:	const char *string - the string in question
// Return:	size_t - size of string

size_t strlen(const char *string)
{
	size_t count = 0;
	while(string[count] != 0)
		count++;

	return count;
}

// dec_to_string: Converts a decimal value to a string
// Param:	uint32_t value - the value
// Param:	char *string - the string
// Return:	char * - pointer to string

char *dec_to_string(uint32_t value, char *string)
{
	if(value <= 9)
	{
		string[0] = (char)value + '0';
		string[1] = 0;
		return string;
	}

	string[10] = 0;		// null terminate
	int i = 9;

	while(i >= 0)
	{
		string[i] = (char)(value % 10) + '0';
		value = value / 10;
		i--;
	}

	// skip preceding zeroes
	i = 0;
	while(string[i] == '0')
		i++;

	return string+i;
}

// hex4_to_string: Converts a hex nibble to a string
// Param:	uint8_t value - the value
// Param:	char *string - string
// Return:	char * - pointer to string

char *hex4_to_string(uint8_t value, char *string)
{
	value &= 0x0F;		// work on low nibble
	if(value < 10)
	{
		value += '0';
	} else
	{
		value += 'A' - 10;
	}

	string[0] = value;
	string[1] = 0;
	return string;
}

// hex8_to_string: Converts a hex byte to a string
// Param:	uint8_t value - the value
// Param:	char *string - string
// Return:	char * - pointer to string

char *hex8_to_string(uint8_t value, char *string)
{
	hex4_to_string(value >> 4, string);
	hex4_to_string(value & 0x0F, string+1);
	return string;
}

// hex16_to_string: Converts a hex word to a string
// Param:	uint16_t value - the value
// Param:	char *string - string
// Return:	char * - pointer to string

char *hex16_to_string(uint16_t value, char *string)
{
	hex8_to_string(value >> 8, string);
	hex8_to_string(value & 0xFF, string+2);
	return string;
}

// hex32_to_string: Converts a hex dword to a string
// Param:	uint32_t value - the value
// Param:	char *string - string
// Return:	char * - pointer to string

char *hex32_to_string(uint32_t value, char *string)
{
	hex16_to_string(value >> 16, string);
	hex16_to_string(value & 0xFFFF, string+4);
	return string;
}

// hex64_to_string: Converts a hex qword to a string
// Param:	uint64_t value - the value
// Param:	char *string - string
// Return:	char * - pointer to string

char *hex64_to_string(uint64_t value, char *string)
{
	hex32_to_string(value >> 32, string);
	hex32_to_string(value & 0xFFFFFFFF, string+8);
	return string;
}

// memset: Sets memory to a value
// Param:	void *dest - destination
// Param:	int value - value to use
// Param:	size_t count - size of memory
// Return:	void * - pointer to destination

void *memset(void *dest, int value, size_t count)
{
	uint8_t val = (uint8_t)(value & 0xFF);
	uint8_t *dest2 = (uint8_t*)(dest);

	size_t i = 0;

	while(i < count)
	{
		dest2[i] = val;
		i++;
	}

	return dest;
}

// memcmp: Compares memory
// Param:	const void *ptr1 - memory
// Param:	const void *ptr2 - memory
// Param:	size_t num - count of bytes
// Return:	int - result of comparison

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	if(!num)
		return 0;

	uint8_t *ptr1_c, *ptr2_c;

	ptr1_c = (uint8_t*)ptr1;
	ptr2_c = (uint8_t*)ptr2;

	size_t i = 0;
	while(i < num)
	{
		if(ptr1_c[i] == ptr2_c[i])
			i++;

		else
		{
			if(ptr1_c[i] > ptr2_c[i])
				return 1;
			else
				return -1;
		}
	}

	return 0;
}

// strcmp(): Compares two strings
// Param:	const char *s1 - string 1
// Param:	const char *s2 - string 2
// Return:	int - result of comparison

int strcmp(const char *s1, const char *s2)
{
	if(strlen(s1) != strlen(s2))
		return -1;

	else
		return memcmp(s1, s2, strlen(s1));
}

// strcpy(): Copies a string
// Param:	char *s1 - destination
// Param:	const char *s2 - source
// Return:	char * - pointer to destination

char *strcpy(char *s1, const char *s2)
{
	memcpy(s1, s2, strlen(s2) + 1);
	return s1;
}

// oct_to_dec(): Converts ASCII octal string to an integer
// Param:	char *string - string
// Return:	size_t - integer

size_t oct_to_dec(char *string)
{
	size_t integer = 0;
	size_t multiplier = 1;
	ssize_t i = (ssize_t)strlen(string) - 1;

	while(i >= 0 && string[i] >= '0' && string[i] <= '7')
	{
		integer += (string[i] - 48) * multiplier;
		multiplier *= 8;
		i--;
	}

	return integer;
}



