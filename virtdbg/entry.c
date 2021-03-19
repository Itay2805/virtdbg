#include <util/except.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <vmx/ept.h>
#include <mm/pmm.h>

#include "virtdbg.h"

__attribute__((section(".init"), used))
void _start(virtdbg_args_t* args) {
    err_t err = NO_ERROR;

    // we ready to roll!
    TRACE("staring up virtdbg");
    TRACE("\tStolen memory: 0x%p-0x%p", args->stolen_memory_base, args->stolen_memory_end);

    CHECK(args->virtdbg_end > args->stolen_memory_base, "Inavlid virtdbg_end");
    CHECK(args->stolen_memory_end > args->virtdbg_end, "Inavlid stolen_memory_end");

    //
    // do initial setup of the hypervisor environment
    //
    init_gdt();
    init_idt();
    init_pmm(args->virtdbg_end, args->stolen_memory_end - args->virtdbg_end);

    //
    // Do the hypervisor setup
    //
    CHECK_AND_RETHROW(init_ept());

cleanup:
    TRACE("We done for now");
    cpu_sleep();
}

