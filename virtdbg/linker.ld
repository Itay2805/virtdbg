ENTRY(_start)

BASE = 0xffffffff80000000;

SECTIONS {
    . = BASE;

    .init :  {
        *(.init*)
    }

    /* Actual kernel. */
    .text :  {
        *(.text*)
    }

    .rodata :  {
        *(.rodata*)
    }

    .data :   {
        *(.data*)
    }

    .bss :  {
        *(.bss*)
        *(COMMON)
        . = ALIGN(4K);
        kernelTop = .;
    }
}
