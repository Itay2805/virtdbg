#include "pmm.h"
#include "string.h"
#include "defs.h"

size_t base = 0;

void pmm_init(struct stivale2_mmap_entry *memmap, size_t memmap_entries) {
    //may need this later
    size_t biggest = 0;

    for (size_t i = 0; i < memmap_entries; i++) {
        if (memmap[i].type != STIVALE2_MMAP_USABLE
         && memmap[i].type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE
         && memmap[i].type != STIVALE2_MMAP_KERNEL_AND_MODULES)
            continue;

        if (memmap[i].length > biggest) {
            biggest = memmap[i].length;
            base = memmap[i].base;
        }
    }
}

void* pmm_alloc_aligned(size_t size, size_t align) {
    base = ALIGN(base, align);
    void* ret = (void*)base;
    base += size;
    return ret;
}

void* pmm_allocz_aligned(size_t size, size_t align) {
    void* ret = pmm_alloc_aligned(size, align);
    memset(ret, 0, size);
    return ret;
}
