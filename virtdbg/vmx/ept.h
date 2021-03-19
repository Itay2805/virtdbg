#ifndef __VIRTDBG_EPT_H__
#define __VIRTDBG_EPT_H__

#include <util/except.h>
#define EPT_WB  (6)
/**
 * The amount of levels in the ept
 */
#define EPT_LEVELS 4
#define EPT_PAGEWALK(n) ((n - 1) << 3)

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

err_t init_ept();

/**
 * Map the given address to the guest, we only do identity mapping
 * with rwx permissions
 */
err_t ept_map(uintptr_t address);

#endif //__VIRTDBG_EPT_H__
