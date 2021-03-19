ENTRY(main)

BASE = 0xffffffff80000000;

SECTIONS {
    . = BASE;

    /* Actual kernel. */
    .text : AT(ADDR(.text) - BASE) {
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata) - BASE) {
        *(.rodata*)
    }

    .data :  AT(ADDR(.data) - BASE) {
        *(.data*)
    }

    .bss : AT(ADDR(.bss) - BASE) {
        *(.bss*)
        *(COMMON)
        . = ALIGN(4K);
        kernelTop = .;
    }
}
