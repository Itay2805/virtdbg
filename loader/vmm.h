#pragma once
#include <stdint.h>
#include <stddef.h>
#include "stivale2.h"
#define PAGE_SIZE 0x1000
#define KERNEL_OFFSET 0xffffffff80000000
#define MEM_PHYS_OFFSET 0xffff800000000000

void vmm_init(uint64_t* pagemap, struct stivale2_mmap_entry *memmap, size_t memmap_entries);
int map_page(uint64_t* pagemap, size_t phys_addr, size_t virt_addr, size_t flags);
