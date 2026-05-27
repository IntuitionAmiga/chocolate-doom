#!/bin/sh
set -eu

script=src/iedoom_build_m68k.sh
linker=src/iedoom_m68k_link.ld
start=src/iedoom_m68k_start.S
runtime=src/iedoom_runtime.c

for required in \
    "$script" \
    "$linker" \
    "$start"
do
    if [ ! -s "$required" ]; then
        echo "missing required M68K build file: $required" >&2
        exit 1
    fi
done

for flag in \
    "-m68020" \
    "-mtune=68020" \
    "-O2" \
    "-ffreestanding" \
    "-nostdlib" \
    "-fno-builtin" \
    "-fno-pic" \
    "-fno-pie" \
    "-fno-stack-protector"
do
    if ! grep -q -- "$flag" "$script"; then
        echo "M68K build script missing required flag: $flag" >&2
        exit 1
    fi
done

if grep -q 'iedoom_boot.asm\|nasm' "$script"; then
    echo "M68K build must not compile the x86 boot/runtime path" >&2
    exit 1
fi

if ! grep -q -- '-print-libgcc-file-name' "$script"; then
    echo "M68K build must link the selected toolchain libgcc helpers" >&2
    exit 1
fi

if ! grep -q -- '"$libgcc"' "$script"; then
    echo "M68K link commands must include libgcc" >&2
    exit 1
fi

missing_tools_path="${TMPDIR:-/tmp}/iedoom-m68k-empty-path"
missing_tools_log="${TMPDIR:-/tmp}/iedoom-m68k-missing-tools.log"
mkdir -p "$missing_tools_path"
if PATH="$missing_tools_path" /bin/sh "$script" build/missing-tools.ie68 >"$missing_tools_log" 2>&1; then
    echo "M68K build must fail when cross tools are missing" >&2
    exit 1
fi
if ! grep -q 'missing M68K cross tools' "$missing_tools_log"; then
    echo "M68K build must report the consolidated missing-tool diagnostic" >&2
    cat "$missing_tools_log" >&2
    exit 1
fi

missing_companion_path="${TMPDIR:-/tmp}/iedoom-m68k-missing-companion-path-$$"
missing_companion_log="${TMPDIR:-/tmp}/iedoom-m68k-missing-companion.log"
mkdir -p "$missing_companion_path"
printf '#!/bin/sh\nexit 0\n' > "$missing_companion_path/m68k-elf-gcc"
chmod +x "$missing_companion_path/m68k-elf-gcc"
ln -sf "$(command -v basename)" "$missing_companion_path/basename"
if PATH="$missing_companion_path" /bin/sh "$script" build/missing-companion.ie68 >"$missing_companion_log" 2>&1; then
    echo "M68K build must fail when companion binutils are missing" >&2
    exit 1
fi
if ! grep -q 'missing M68K cross tools' "$missing_companion_log"; then
    echo "M68K build must report the consolidated missing-companion diagnostic" >&2
    cat "$missing_companion_log" >&2
    exit 1
fi

if ! grep -q 'DIEDOOM_M68K' "$script"; then
    echo "M68K build must define IEDOOM_M68K" >&2
    exit 1
fi

if ! grep -q 'PROGRAM_START = 0x1000' "$linker"; then
    echo "M68K linker script must place code at 0x1000" >&2
    exit 1
fi

for symbol in MMIO_START MMIO_END HEAP_BASE STACK_TOP __bss_start __bss_end __heap_base __stack_top; do
    if ! grep -q "$symbol" "$linker"; then
        echo "M68K linker script missing $symbol" >&2
        exit 1
    fi
done

if ! grep -q 'ASSERT(. <= MMIO_START' "$linker"; then
    echo "M68K linker script must reject loadable sections that overlap MMIO" >&2
    exit 1
fi

if ! grep -q 'MAX(., MMIO_END)' "$linker"; then
    echo "M68K .bss must be placed outside the fixed MMIO aperture" >&2
    exit 1
fi

if ! grep -q '.section .text.start' "$start"; then
    echo "M68K startup must use the pinned .text.start section when the toolchain supports it" >&2
    exit 1
fi

if ! grep -q 'iedoom_m68k_start.o" $objects' "$script"; then
    echo "M68K build must link the startup object before other objects" >&2
    exit 1
fi

if ! grep -q 'move.l  #__stack_top,%sp' "$start"; then
    echo "M68K startup must initialize A7 from __stack_top" >&2
    exit 1
fi

if ! grep -q 'jsr     iedoom_entry' "$start"; then
    echo "M68K startup must jump through iedoom_entry" >&2
    exit 1
fi

if ! grep -q 'clr.b   (%a0)+' "$start"; then
    echo "M68K startup must clear .bss" >&2
    exit 1
fi

if ! grep -q 'stop    #0x2700' "$start"; then
    echo "M68K startup must provide a halt path" >&2
    exit 1
fi

if ! grep -q 'SDL_BYTEORDER SDL_BIG_ENDIAN' src/iedoom/include/SDL_endian.h; then
    echo "M68K build must force SDL little-endian swap helpers onto big-endian mode" >&2
    exit 1
fi

if ! grep -q 'defined(IEDOOM_M68K)' "$runtime"; then
    echo "runtime must isolate M68K call/halt assembly from x86 assembly" >&2
    exit 1
fi

out="${1:-build/iedoom.ie68}"
if command -v m68k-elf-gcc >/dev/null 2>&1 \
 || command -v m68k-atari-mint-gcc >/dev/null 2>&1 \
 || command -v m68k-linux-gnu-gcc-13 >/dev/null 2>&1 \
 || command -v m68k-linux-gnu-gcc >/dev/null 2>&1 \
 || command -v m68k-suse-linux-gcc >/dev/null 2>&1; then
    sh "$script" "$out"
    elf="${out%.ie68}.elf"
    if [ ! -s "$out" ] || [ ! -s "$elf" ]; then
        echo "M68K build did not emit .ie68 and .elf artifacts" >&2
        exit 1
    fi
    prefix=$(od -An -tx1 -N6 "$out" | tr -d ' \n')
    if [ "$prefix" != "2e7c00ff0000" ]; then
        echo "M68K flat image does not start at _start: $prefix" >&2
        exit 1
    fi
else
    echo "M68K cross compiler not found; structural tests passed, compile skipped"
fi

echo "iedoom_build_m68k tests passed"
