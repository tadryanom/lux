
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

typedef uint32_t lock_t;

void acquire_lock(lock_t *);
void release_lock(lock_t *);



