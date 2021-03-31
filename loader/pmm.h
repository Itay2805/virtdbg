#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>

void pmm_init(struct stivale2_mmap_entry *memmap, size_t memmap_entries);
void* pmm_alloc_aligned(size_t size, size_t align);
void* pmm_allocz_aligned(size_t size, size_t align);
