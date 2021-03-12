#ifndef __VIRTDBG_VIRTDBG_H__
#define __VIRTDBG_VIRTDBG_H__

#include <stdint.h>

typedef struct virtdbg_args {
    // we assume that the base of the binary is at the
    // base of the stolen memory, and that there is more
    // space at the end of the binary itself that can be
    // used for dynamic memory allocation
    uintptr_t stolen_memory_base;

    // this should point to the area after the virtdbg binary
    // AND after the page tables and stack that the loader
    // allocated to the hypervisor
    uintptr_t virtdbg_end;

    // this is the end of the stolen memory, anything between the prev
    // variable to this can be used by the hypervisor for whatever
    // dynamic memory it may need
    uintptr_t stolen_memory_end;

    // the return address of the loader, once vmx is initialized this is
    // where the hypervisor will load everything to
    uintptr_t return_address;
} virtdbg_args_t;

#endif //__VIRTDBG_VIRTDBG_H__
