
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>
#include <boot.h>

#if __i386__
#define PMM_BITMAP_SIZE			0x100000
#endif

#define PAGE_SIZE			4096
#define PAGE_SIZE_SHIFT			12		// 12 bits for 4096

#define PAGE_PRESENT			0x01
#define PAGE_RW				0x02
#define PAGE_USER			0x04
#define PAGE_UNCACHEABLE		0x10

#if __i386__
#define KERNEL_HEAP			0xD8000000
#define HW_FRAMEBUFFER			0xF0000000
#define SW_FRAMEBUFFER			0xF4000000
#define HEAP_ALIGNMENT			16		// SSE-aligned
#endif

extern uint64_t total_memory, usable_memory;
extern uint8_t *pmm_bitmap;

// Generic Functions
void *kmalloc(size_t);
void *kcalloc(size_t, size_t);
void *krealloc(void *, size_t);
void kfree(void *);

void mm_init(multiboot_info_t *);

// Physical Memory Manager
void pmm_init(multiboot_info_t *);
void pmm_mark_used(size_t, size_t);
void pmm_mark_free(size_t, size_t);
uint8_t pmm_is_page_free(size_t);
size_t pmm_find_range(size_t);
size_t pmm_alloc(size_t);

// Virtual Memory Manager
size_t *page_directory, *page_tables;
void vmm_init();
size_t vmm_get_page(size_t);
void vmm_map(size_t, size_t, size_t, uint8_t);
void vmm_unmap(size_t, size_t);
size_t vmm_find_range(size_t, size_t);
size_t vmm_alloc(size_t, size_t, uint8_t);
void vmm_free(size_t, size_t);
size_t vmm_request_map(size_t, size_t, uint8_t);



