#ifndef __VIRTDBG_VIRTDBG_H__
#define __VIRTDBG_VIRTDBG_H__

#include <stdint.h>

typedef struct descriptor {
    uint16_t size;
    uint64_t address;
} __attribute__((packed)) descriptor_t;

typedef struct guest_state {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rflags;
} __attribute__((packed)) guest_state_t;

typedef struct initial_guest_state {
    uint64_t apic_id;
    guest_state_t gprstate;
    uint64_t rip;
    uint64_t rsp;
    uint64_t dr7;
    uint64_t rflags;
    uint64_t efer;
    uint64_t cr0;
    uint64_t cr4;
    uint64_t cr3;
    uint64_t ss;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    uint64_t cs;
    descriptor_t gdt;
    descriptor_t idt;
} __attribute__((packed)) initial_guest_state_t;

/**
 * This is passed to the hypervisor upon initialization, it does not have
 * to be in the hypervisor stolen memory, since once we enter the guest we
 * assume this will be overridden.
 *
 * on the other hand we do need the page tables and initial stack to be inside
 * the stolen memory since we assume it will not be changed once we enter the guest.
 * of course if the loader makes sure it won't be overridden then no problem should come
 * from not putting it in the stolen memory range.
 */
typedef struct virtdbg_args {
    // we assume that the base of the binary is at the
    // base of the stolen memory, and that there is more
    // space at the end of the binary itself that can be
    // used for dynamic memory allocation
    uint64_t stolen_memory_base;

    // this should point to the area after the virtdbg binary
    // AND after the page tables and stack that the loader
    // allocated to the hypervisor
    uint64_t virtdbg_end;

    // this is the end of the stolen memory, anything between the prev
    // variable to this can be used by the hypervisor for whatever
    // dynamic memory it may need
    uint64_t stolen_memory_end;

    // the initial state of the guest that the virtdbg should create, this
    // should have a state for every initialized cpu, if the state is not
    // present virtdbg will assume that core has not been activated yet
    uint8_t initial_guest_state_count;
    initial_guest_state_t* initial_guest_state;
} __attribute__((packed)) virtdbg_args_t;

#endif //__VIRTDBG_VIRTDBG_H__
