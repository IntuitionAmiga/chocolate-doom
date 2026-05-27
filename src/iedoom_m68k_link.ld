OUTPUT_FORMAT("elf32-m68k")
OUTPUT_ARCH(m68k)
ENTRY(_start)

PROGRAM_START = 0x1000;
MMIO_START = 0x000F0000;
MMIO_END = 0x00100000;
HEAP_BASE = 0x02000000;
STACK_TOP = 0x00FF0000;

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = PROGRAM_START;
    .text : {
        KEEP(*(.text.start))
        KEEP(*(.text.entry))
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    .data : {
        *(.data .data.*)
    } :data

    __load_end_before_mmio = .;
    ASSERT(. <= MMIO_START, "IEDoom M68K loadable sections overlap the IE MMIO aperture")

    . = MAX(., MMIO_END);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data

    __heap_base = HEAP_BASE;
    __stack_top = STACK_TOP;

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame)
    }
}
