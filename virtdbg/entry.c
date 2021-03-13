#include <util/except.h>
#include <stdint.h>
#include <util/trace.h>
#include <arch/cpu.h>
#include <mm/pmm.h>
#include <vmx/ept.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include "virtdbg.h"
#include "elf.h"

extern elf64_rela_t _RELOCATIONS[];
extern uint64_t _RELOCATIONS_SIZE;

static void relocate(uintptr_t base) {
    elf64_rela_t* rel = _RELOCATIONS;
    size_t relsz = _RELOCATIONS_SIZE;

    // do the relocations
    while (relsz > 0) {
        switch(ELF64_R_TYPE(rel->info)) {
            case R_X86_64_NONE: break;

            case R_X86_64_RELATIVE: {
                *(unsigned long*)(base + rel->offset) += base;
            } break;

            default: {
                WARN("unknown relocation %d", ELF64_R_TYPE(rel->info));
            } break;
        }

        // next entry
        rel++;
        relsz -= sizeof(elf64_rela_t);
    }
}

__attribute__((section(".init"), used))
void _start(virtdbg_args_t* args) {
    err_t err = NO_ERROR;

    // first of all do the relocation so we can
    // start doing everything else
    relocate(args->stolen_memory_base);
    memory_barrier();

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

