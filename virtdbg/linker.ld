ENTRY(_start)

PHDRS {
  none   PT_NULL FLAGS(0);
  text   PT_LOAD FLAGS(0);
}

SECTIONS
{
    . = 0x0;

    .init : {
        *(.init)
        *(.init.*)
    }:text

    .text : {
        *(.text)
        *(.text.*)
    }:text

    .data : {
        _RELOCATIONS = .;
        *(.rela)
        *(.rela.*)
        *(.rela*)
        _RELOCATIONS_SIZE = .;
        QUAD(. - _RELOCATIONS);
        *(.got)
        *(.rodata*)
        *(.data*)
        *(.bss)
    }:text
}