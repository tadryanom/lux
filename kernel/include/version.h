
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#if __i386__
#define KERNEL			"lux32"
#endif

#if __x86_64__
#define KERNEL			"lux64"
#endif

#define VERSION			KERNEL " 0.01 (compiled " __DATE__ " " __TIME__ ")"
#define COPYRIGHT		"copyright (c) 2018 by Omar Mohammad"


