#include <util/except.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <vmx/ept.h>
#include <drivers/serial.h>
#include <vmx/ept.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <vmx/vmm.h>
#include <mm/pmm.h>
#include "virtdbg.h"
#include <arch/io.h>
#include <gdb/gdb.h>

__attribute__((section(".init"), used))
void _start(virtdbg_args_t* args) {
    err_t err = NO_ERROR;

    serial_init();
    TRACE("staring up virtdbg");

    // we ready to roll!
    TRACE("\tStolen memory: 0x%p-0x%p, base: %p", args->stolen_memory_base, args->stolen_memory_end, args->virtdbg_end);

    CHECK(args->virtdbg_end > args->stolen_memory_base, "Inavlid virtdbg_end");
    CHECK(args->stolen_memory_end > args->virtdbg_end, "Inavlid stolen_memory_end");

    //
    // do initial setup of the hypervisor environment
    //
    init_gdt();
    init_idt();
    TRACE("staring up virtdbg");
 
    init_pmm(args->virtdbg_end, args->stolen_memory_end - args->virtdbg_end);

    //
    // register the gdb stub
    //
    init_kernel_gdb();

    //
    // Do the hypervisor setup
    //
    CHECK_AND_RETHROW(init_ept());

    TRACE("ept initialized");

    CHECK_AND_RETHROW(vmxon());

    vmcs_t v;
    init_vmcs(&v, &args->initial_guest_state[0]);

cleanup:
    TRACE("We done for now");
    cpu_sleep();
}

