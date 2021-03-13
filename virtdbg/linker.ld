OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS {
    . = 0;

    .init : {
        *(.init)
        *(.init.*)
    }

    .text : {
        *(.text)
        *(.text.*)
    }

    . = ALIGN(4096);
    .dynamic : {
        _DYNAMIC = .;
        *(.dynamic)

        /* just in case there is no dynamic section add a dummy dynamic entry */
        QUAD(0x0)
        QUAD(0x0)
    }

    . = ALIGN(4096);
    .data : {
        *(.rodata*)
        *(.data*)
        *(.bss)
    }
}