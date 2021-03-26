#include "vmm.h"
#include "pmm.h"
#include "defs.h"

void vmm_init(uint64_t* pagemap, struct stivale2_mmap_entry *memmap, size_t memmap_entries) {
    for (uintptr_t p = 0; p < 0x100000000; p += PAGE_SIZE) {
        map_page(pagemap, p, p, 0x03);
    }

    for (size_t i = 0; i < memmap_entries; i++) {
        for (uintptr_t p = 0; p < memmap[i].length; p += PAGE_SIZE) {
            map_page(pagemap, p + memmap[i].base, p + memmap[i].base, 0x03);
            map_page(pagemap, p + memmap[i].base, p + memmap[i].base + MEM_PHYS_OFFSET, 0x03);
        }
    }

    asm volatile (
        "mov %0, %%cr3"
        :
        : "r" (pagemap)
        : "memory"
    );
}

int map_page(uint64_t* pagemap, size_t phys_addr, size_t virt_addr, size_t flags) {
    size_t pml4_entry = (virt_addr & ((size_t)0x1ff << 39)) >> 39;
    size_t pdpt_entry = (virt_addr & ((size_t)0x1ff << 30)) >> 30;
    size_t pd_entry = (virt_addr & ((size_t)0x1ff << 21)) >> 21;
    size_t pt_entry = (virt_addr & ((size_t)0x1ff << 12)) >> 12;

    uint64_t *pdpt, *pd, *pt;

    if (pagemap[pml4_entry] & 0x1) {
        pdpt = (uint64_t *)((pagemap[pml4_entry] & 0xfffffffffffff000));
    } else {
        pdpt = (uint64_t *)((size_t)pmm_allocz_aligned(PAGE_SIZE, PAGE_SIZE));
        pagemap[pml4_entry] = (uint64_t)((size_t)pdpt) | 0b111;
    }

    if (pdpt[pdpt_entry] & 0x1) {
        pd = (uint64_t *)((pdpt[pdpt_entry] & 0xfffffffffffff000));
    } else {
        pd = (uint64_t *)((size_t)pmm_allocz_aligned(PAGE_SIZE, PAGE_SIZE));
        pdpt[pdpt_entry] = (uint64_t)((size_t)pd) | 0b111;
    }

    if (pd[pd_entry] & 0x1) {
        pt = (uint64_t *)((pd[pd_entry] & 0xfffffffffffff000));
    } else {
        pt = (uint64_t *)((size_t)pmm_allocz_aligned(PAGE_SIZE, PAGE_SIZE));
        pd[pd_entry] = (uint64_t)((size_t)pt) | 0b111;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}
