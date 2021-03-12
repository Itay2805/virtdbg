#include <util/except.h>
#include <stdint.h>
#include <util/trace.h>
#include <arch/cpu.h>
#include <mm/mm.h>
#include <vmx/ept.h>
#include "virtdbg.h"
#include "elf.h"

static void relocate(uintptr_t base) {
    elf64_rela_t* rel = 0;
    size_t relsz = 0;
    size_t relent = 0;

    // get the dynamic symbol nicely
    elf64_dyn_t* dynamic = NULL;
    asm ("lea _DYNAMIC(%%rip), %0" : "=r"(dynamic));

    // search for relocation information
    for (int i = 0; dynamic[i].tag != DT_NULL; i++) {
        switch (dynamic[i].tag) {
            case DT_RELA: {
                rel = (elf64_rela_t*)dynamic[i].ptr;
            } break;

            case DT_RELASZ: {
                relsz = dynamic[i].val;
            } break;

            case DT_RELAENT: {
                relent = dynamic[i].val;
            } break;

            default: break;
        }
    }

    if (rel == NULL && relsz == 0 && relent == 0) {
        // no relocations to do
        return;
    }

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
        rel = (elf64_rela_t*)((uintptr_t) rel + relent);
        relsz -= relent;
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

    // setup the pmm
    TRACE("Initializing PMM");
    init_pmm(args->virtdbg_end, args->stolen_memory_end - args->virtdbg_end);

    // setup the ept
    TRACE("Initializing EPT");
    CHECK_AND_RETHROW(init_ept());

cleanup:
    cpu_sleep();
}

