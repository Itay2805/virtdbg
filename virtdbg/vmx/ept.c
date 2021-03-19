#include <sync/lock.h>
#include <mm/pmm.h>
#include "ept.h"

/**
 * The lock of the ept
 */
static lock_t m_ept_lock = INIT_LOCK();

/**
 * The root of the ept page tables
 */
ept_entry_t* g_root_pa;

err_t init_ept() {
    err_t err = NO_ERROR;

    TRACE("Initializing EPT");

    // allocate the top level page
    g_root_pa = pallocz_aligned(4096, 0x1000);
    CHECK_ERROR(g_root_pa != NULL, ERROR_OUT_OF_RESOURCES);

cleanup:
    return err;
}

void invept() {
    //invept the entire EPT, since doing so on a single address requires the VPCID which we don't need
    struct descr {
        uint64_t eptp;
        uint64_t gpa;
    };

    struct descr d = {(uintptr_t)g_root_pa | 6 | (4 - 1) << 3, 0};
    uint64_t type = 1;

    asm volatile("invept %1, %0" : : "r"(type), "m"(d) : "memory");
}

err_t ept_map(uintptr_t address) {
    err_t err = NO_ERROR;
    lock(&m_ept_lock);

	int pteIdx   = (((address) >> 12) & 0x1ff);

    ept_entry_t* cur = (ept_entry_t*)g_root_pa;
    for (int i = 4; i > 1; i--) {
        size_t idx = (((address) >> (12 + 9 * (i - 1))) & 0x1ff);
        if (!(cur[idx].r)) {
            uintptr_t alloc = (uintptr_t)pallocz_aligned(0x1000, 0x1000);
            CHECK_ERROR(alloc != 0, ERROR_OUT_OF_RESOURCES);
            cur[idx].frame = (alloc >> 12);
            cur[idx].r = 1;
            cur[idx].w = 1;
            cur[idx].x = 1;
            cur = (ept_entry_t*)alloc;
        } else {
            cur = (ept_entry_t*)(((uintptr_t)cur[idx].frame) << 12);
        }
    }

    cur[pteIdx].frame = address >> 12;
    cur[pteIdx].r = 1;
    cur[pteIdx].w = 1;
    cur[pteIdx].x = 1;
    cur[pteIdx].mem_type = EPT_WB;

    invept();

cleanup:
    unlock(&m_ept_lock);
    return err;
}

