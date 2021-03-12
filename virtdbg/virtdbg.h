#ifndef __VIRTDBG_VIRTDBG_H__
#define __VIRTDBG_VIRTDBG_H__

#include <stdint.h>

typedef struct virtdbg_args {
    // we assume that the base of the binary is at the
    // base of the stolen memory, and that there is more
    // space at the end of the binary itself that can be
    // used for dynamic memory allocation
    uintptr_t stolen_memory_base;
    uintptr_t virtdbg_end;
    uintptr_t stolen_memory_end;

    // the return address of the loader
    uintptr_t return_address;
} virtdbg_args_t;

#endif //__VIRTDBG_VIRTDBG_H__
