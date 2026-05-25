OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

PROGRAM_START = 0x1000;
MMIO_START = 0x000F0000;
MMIO_END = 0x00100000;

PHDRS
{
    reset PT_LOAD FLAGS(5);
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0;
    .reset : {
        KEEP(*(.reset))
    } :reset

    . = PROGRAM_START;
    .text : {
        KEEP(*(.text.entry))
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    .data : {
        *(.data .data.*)
    } :data

    . = MAX(., MMIO_END);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame)
    }
}
