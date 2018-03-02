
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <rand.h>
#include <time.h>

static uint64_t seed = 0;

// rand(): Generates a random number
// Param:	Nothing
// Return:	unsigned int - random number

unsigned int rand()
{
	if(seed <= 123456789)
		seed = get_time();

	seed = seed * 1103515245 + 12345;
	return (unsigned int)(seed / ((RAND_MAX+1)*2)) % (RAND_MAX+1);
}





