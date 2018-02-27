
/*
 * lux OS
 * copyright (c) 2018 by Omar Mohammad
 * This runtime does what compiler-rt libgcc do
 */

#include <stdint.h>

// 64-bit divide and mod, stores the quotient in *q and the remainder in *r
void __udivmod64(uint64_t n, uint64_t d, uint64_t *q, uint64_t *r)
{
	if(n == 0)
	{
		q[0] = 0;
		r[0] = 0;
		return;
	}

	if(d == 0)
	{
		// make a divide by zero exception
		volatile uint32_t zero = 0;
		volatile uint32_t n32 = (volatile uint32_t)n;
		volatile uint32_t *ptr = (volatile uint32_t*)q;
		ptr[0] = n32 / zero;
		return;
	}

	// determine how many bits are used
	int8_t bit = 63;

	while(((n >> bit) & 1) == 0)
		bit--;

	// now we know the highest bit, so work
	q[0] = 0;
	r[0] = 0;

	while(bit > 0 || bit == 0)
	{
		r[0] <<= 1;
		r[0] |= ((n >> bit) & 1);

		if(r[0] >= d)
		{
			r[0] -= d;
			q[0] |= (1 << bit);
		}

		bit--;
	}
}

// 64-bit divide
uint64_t __udivdi3(uint64_t n, uint64_t d)
{
	uint64_t q, r;
	__udivmod64(n, d, &q, &r);
	return q;
}

// 64-bit mod
uint64_t __umoddi3(uint64_t n, uint64_t d)
{
	uint64_t q, r;
	__udivmod64(n, d, &q, &r);
	return r;
}



