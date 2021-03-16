#ifndef __VIRTDBG_MM_H__
#define __VIRTDBG_MM_H__

#include <stdint.h>
#include <stddef.h>

void init_pmm(uintptr_t base, size_t size);

void* palloc(size_t size);

void pfree(void* ptr, size_t size);

void* pallocz(size_t size);

void* pallocz_aligned(size_t size, size_t align);

void* palloc_aligned(size_t size, size_t align);

#endif //__VIRTDBG_MM_H__
