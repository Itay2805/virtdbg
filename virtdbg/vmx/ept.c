#include <sync/lock.h>
#include <mm/pmm.h>
#include "ept.h"

typedef struct ept_entry {
    uint64_t r : 1;
    uint64_t w : 1;
    uint64_t x : 1;
    uint64_t mem_type : 3;
    uint64_t ignore_pat : 1;
    uint64_t _reserved0 : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t linear_x : 1;
    uint64_t _reserved1 : 1;
    uint64_t frame : 40;
    uint64_t _reserved2 : 8;
    uint64_t super_visor_shadow : 1;
    uint64_t spp : 1;
    uint64_t _reserved3 : 1;
    uint64_t suppress_ve : 1;
} __attribute__((packed)) ept_entry_t;

/**
 * The lock of the ept
 */
static lock_t m_ept_lock = INIT_LOCK();

/**
 * The root of the ept page tables
 */
static ept_entry_t* m_root_pa;

/**
 * The amount of levels in the ept
 */
#define EPT_LEVELS 4

err_t init_ept() {
    err_t err = NO_ERROR;

    TRACE("Initializing EPT");

    // allocate the top level page
    m_root_pa = pallocz(4096);
    CHECK_ERROR(m_root_pa != NULL, ERROR_OUT_OF_RESOURCES);

cleanup:
    return err;
}

err_t ept_map(uintptr_t address) {
    err_t err = NO_ERROR;

    lock(&m_ept_lock);

    ept_entry_t* curr = m_root_pa;
    for (size_t i = EPT_LEVELS; i >= 0; i--) {
        // get the current index
        size_t index = ((address >> ((9 * (i - 1)) + 12)) & 0x1FF);

        if (!curr[index].r) {
            // the entry is not present, create it
            void* new_frame = pallocz(4096);
            CHECK_ERROR(new_frame != NULL, ERROR_OUT_OF_RESOURCES);

            curr[index].frame = (uint64_t)new_frame >> 12;
            curr[index].r = 1;
            curr[index].w = 1;
            curr[index].x = 1;
        }

        // get the next level
        curr = (ept_entry_t*)((uintptr_t)curr[index].frame << 12);
    }

cleanup:
    unlock(&m_ept_lock);
    return err;
}
