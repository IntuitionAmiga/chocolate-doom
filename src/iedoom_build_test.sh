#!/bin/sh
set -eu

script=src/iedoom_build.sh
linker=src/iedoom_link.ld
boot=src/iedoom_boot.asm

for required in \
    "$script" \
    "$linker" \
    "$boot"
do
    if [ ! -s "$required" ]; then
        echo "missing required x86 build file: $required" >&2
        exit 1
    fi
done

for symbol in MMIO_START MMIO_END HEAP_BASE STACK_TOP __bss_start __bss_end __heap_base __stack_top; do
    if ! grep -q "$symbol" "$linker"; then
        echo "x86 linker script missing $symbol" >&2
        exit 1
    fi
done

if ! grep -q 'ASSERT(. <= MMIO_START' "$linker"; then
    echo "x86 linker script must reject loadable sections that overlap MMIO" >&2
    exit 1
fi

if ! grep -q 'ORIGIN(post_mmio)' "$linker"; then
    echo "x86 .bss must be placed outside the fixed MMIO aperture" >&2
    exit 1
fi

if ! grep -q 'mov     esp, STACK_TOP' "$boot"; then
    echo "x86 startup must initialize ESP from STACK_TOP" >&2
    exit 1
fi

if ! grep -q 'rep     stosb' "$boot"; then
    echo "x86 startup must clear .bss" >&2
    exit 1
fi

out="${1:-build/iedoom.ie86}"
sh "$script" "$out"
elf="${out%.ie86}.elf"

if [ ! -s "$out" ]; then
    echo "missing iedoom image: $out" >&2
    exit 1
fi

prefix=$(od -An -tx1 -N8 "$out" | tr -d ' \n')
if [ "$prefix" != "bc0000ff00fc31c0" ]; then
    echo "unexpected iedoom reset bytes: $prefix" >&2
    exit 1
fi

entry_byte=$(od -An -tx1 -j4096 -N1 "$out" | tr -d ' \n')
if [ "$entry_byte" != "e8" ]; then
    echo "iedoom PROGRAM_START does not call C entry: $entry_byte" >&2
    exit 1
fi

if ! nm "$elf" | grep -q ' T D_ProcessEvents$'; then
    echo "iedoom image is not linked with Doom's real d_main.c" >&2
    exit 1
fi

bss_end=$(nm "$elf" | awk '$3 == "__bss_end" { print "0x" $1 }')
if [ -z "$bss_end" ] || [ "$((bss_end))" -ge "$((0x00ff0000))" ]; then
    echo "iedoom .bss overlaps STACK_TOP: $bss_end" >&2
    exit 1
fi

bss_start=$(nm "$elf" | awk '$3 == "__bss_start" { print "0x" $1 }')
if [ -z "$bss_start" ] || { [ "$((bss_start))" -lt "$((0x00100000))" ] && [ "$((bss_end))" -gt "$((0x000f0000))" ]; }; then
    echo "iedoom .bss places guest globals in the fixed IE MMIO aperture" >&2
    exit 1
fi

heap_base=$(nm "$elf" | awk '$3 == "__heap_base" { print "0x" $1 }')
stack_top=$(nm "$elf" | awk '$3 == "__stack_top" { print "0x" $1 }')
if [ "$((heap_base))" -ne "$((0x02000000))" ] || [ "$((stack_top))" -ne "$((0x00ff0000))" ]; then
    echo "iedoom heap/stack symbols are wrong: heap=$heap_base stack=$stack_top" >&2
    exit 1
fi

load_end=$(nm "$elf" | awk '$3 == "__load_end_before_mmio" { print "0x" $1 }')
if [ -z "$load_end" ] || [ "$((load_end))" -gt "$((0x000f0000))" ]; then
    echo "iedoom loadable sections overlap fixed IE MMIO aperture: $load_end" >&2
    exit 1
fi

if nm --undefined-only "$elf" | grep .; then
    echo "iedoom image has unresolved symbols" >&2
    exit 1
fi

if objdump -d "$elf" | grep -Eiq '\<xmm[0-9]+\>|movdqu|movaps|movups'; then
    echo "iedoom image contains SSE/XMM instructions unsupported by the IE x86 core" >&2
    exit 1
fi

if ! grep -q 'ie_present_framebuffers\[2\]' src/i_video.c; then
    echo "IEDoom video must use alternate present buffers instead of live draw-buffer scanout" >&2
    exit 1
fi

if ! grep -q 'IE_MMIO_Write32(IE_VIDEO_FB_BASE' src/i_video.c; then
    echo "IEDoom video must flip VIDEO_FB_BASE from I_FinishUpdate" >&2
    exit 1
fi

echo "iedoom_build tests passed"
