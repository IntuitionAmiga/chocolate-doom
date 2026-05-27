OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

PROGRAM_START = 0x1000;
MMIO_START = 0x000F0000;
MMIO_END = 0x00100000;
HEAP_BASE = 0x02000000;
STACK_TOP = 0x00FF0000;

MEMORY
{
    pre_mmio (rwx) : ORIGIN = 0x00000000, LENGTH = MMIO_START
    post_mmio (rw) : ORIGIN = MMIO_END, LENGTH = STACK_TOP - MMIO_END
}

PHDRS
{
    reset PT_LOAD FLAGS(5);
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    .reset 0 : {
        KEEP(*(.reset))
    } > pre_mmio :reset

    .text PROGRAM_START : {
        KEEP(*(.text.entry))
        *(.text .text.*)
    } > pre_mmio :text

    .rodata : {
        *(.rodata .rodata.*)
    } > pre_mmio :text

    .data : {
        *(.data .data.*)
    } > pre_mmio :data

    /*
     * Allocated orphan sections placed after .data inherit pre_mmio. If any
     * such loadable section reaches the fixed MMIO aperture, ld reports a
     * pre_mmio overflow before this script moves .bss above MMIO_END.
     */
    __load_end_before_mmio = .;
    ASSERT(. <= MMIO_START, "IEDoom x86 loadable sections overlap the IE MMIO aperture")

    .bss ORIGIN(post_mmio) (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } > post_mmio :data

    __heap_base = HEAP_BASE;
    __stack_top = STACK_TOP;

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame)
    }
}
