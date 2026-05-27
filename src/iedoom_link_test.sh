#!/bin/sh
set -eu

tmp="${TMPDIR:-/tmp}/iedoom-link-test"
elf="$tmp/iedoom.elf"
bin="$tmp/iedoom.ie86"
libgcc=$(cc -m32 -print-libgcc-file-name)

rm -rf "$tmp"
mkdir -p "$tmp"

nasm -f elf32 -I ../IntuitionEngine/sdk/include/ \
    -o "$tmp/iedoom_boot.o" src/iedoom_boot.asm
cc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
    -fno-asynchronous-unwind-tables -DIEDOOM_GUEST -c src/iedoom_runtime.c \
    -o "$tmp/iedoom_runtime.o"
cc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
    -fno-asynchronous-unwind-tables -DIEDOOM_GUEST -I/tmp/choc-ie-config -Isrc -I. \
    -c src/iedoom_main.c -o "$tmp/iedoom_main.o"
cc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
    -fno-asynchronous-unwind-tables -DIEDOOM_GUEST -c src/iedoom_link_stubs.c \
    -o "$tmp/iedoom_link_stubs.o"
ld -m elf_i386 -nostdlib -T src/iedoom_link.ld \
    -o "$elf" "$tmp/iedoom_boot.o" "$tmp/iedoom_runtime.o" \
    "$tmp/iedoom_main.o" "$tmp/iedoom_link_stubs.o" "$libgcc"
objcopy -O binary "$elf" "$bin"

size=$(wc -c < "$bin")
if [ "$size" -lt 4097 ]; then
    echo "linked image does not include PROGRAM_START payload" >&2
    exit 1
fi

prefix=$(od -An -tx1 -N8 "$bin" | tr -d ' \n')
if [ "$prefix" != "bc0000ff00fc31c0" ]; then
    echo "unexpected linked trampoline bytes: $prefix" >&2
    exit 1
fi

entry_addr=$(nm -n "$elf" | awk '$3 == "iedoom_entry" { print $1 }')
if [ "$entry_addr" != "00001000" ]; then
    echo "iedoom_entry is not linked at PROGRAM_START: $entry_addr" >&2
    exit 1
fi

bss_start=$(nm -n "$elf" | awk '$3 == "__bss_start" { print $1 }')
bss_end=$(nm -n "$elf" | awk '$3 == "__bss_end" { print $1 }')
bss_probe=$(nm -n "$elf" | awk '$3 == "iedoom_bss_probe" { print $1 }')
heap_base=$(nm -n "$elf" | awk '$3 == "__heap_base" { print $1 }')
stack_top=$(nm -n "$elf" | awk '$3 == "__stack_top" { print $1 }')
load_end=$(nm -n "$elf" | awk '$3 == "__load_end_before_mmio" { print $1 }')
if [ -z "$bss_start" ] || [ -z "$bss_end" ] || [ -z "$bss_probe" ]; then
    echo "linked image is missing bss symbols" >&2
    exit 1
fi
if [ "$heap_base" != "02000000" ] || [ "$stack_top" != "00ff0000" ]; then
    echo "linked image has wrong heap/stack symbols: heap=$heap_base stack=$stack_top" >&2
    exit 1
fi
if [ -z "$load_end" ] || [ "$((0x$load_end))" -gt "$((0x000f0000))" ]; then
    echo "linked image loadable sections overlap MMIO: load_end=$load_end" >&2
    exit 1
fi
if [ "$bss_start" = "$bss_end" ]; then
    echo "linked image has an empty bss range" >&2
    exit 1
fi
if ! objdump -dr "$tmp/iedoom_boot.o" | grep -q "__bss_start"; then
    echo "reset stub does not reference __bss_start" >&2
    exit 1
fi
if ! objdump -dr "$tmp/iedoom_boot.o" | grep -q "__bss_end"; then
    echo "reset stub does not reference __bss_end" >&2
    exit 1
fi
if ! objdump -dr "$tmp/iedoom_boot.o" | grep -q "mov.*esp"; then
    echo "reset stub does not initialize ESP" >&2
    exit 1
fi
if ! nm -n "$elf" | awk '{ print $3 }' | grep -qx "iedoom_main"; then
    echo "linked image is missing iedoom_main" >&2
    exit 1
fi
if ! objdump -dr "$tmp/iedoom_runtime.o" | grep -q "iedoom_main"; then
    echo "iedoom_entry does not call iedoom_main" >&2
    exit 1
fi

entry_byte=$(od -An -tx1 -j4096 -N1 "$bin" | tr -d ' \n')
if [ "$entry_byte" != "e8" ]; then
    echo "iedoom_entry does not begin with call: $entry_byte" >&2
    exit 1
fi

cat > "$tmp/mmio_overlap.asm" <<'EOF'
section .text progbits alloc exec nowrite align=1
global mmio_overlap
mmio_overlap:
    hlt
section .data progbits alloc write align=1
db 1
section .orphan_after_data progbits alloc write align=1
times 0xF0000 db 0
EOF
nasm -f elf32 -o "$tmp/mmio_overlap.o" "$tmp/mmio_overlap.asm"
if ld -m elf_i386 -nostdlib -T src/iedoom_link.ld -o "$tmp/mmio_overlap.elf" "$tmp/mmio_overlap.o" 2>"$tmp/mmio_overlap.err"; then
    echo "linker accepted a loadable x86 section crossing MMIO" >&2
    exit 1
fi
if ! grep -q "overlap the IE MMIO aperture" "$tmp/mmio_overlap.err" \
 && ! grep -q "pre_mmio.*overflowed" "$tmp/mmio_overlap.err"; then
    echo "linker failed for the wrong reason when testing MMIO orphan overlap" >&2
    cat "$tmp/mmio_overlap.err" >&2
    exit 1
fi

if ld -m elf_i386 -nostdlib -T src/iedoom_link.ld -o "$tmp/unresolved.elf" "$tmp/iedoom_boot.o" 2>"$tmp/unresolved.err"; then
    echo "linker accepted unresolved startup symbols" >&2
    exit 1
fi
if ! grep -q "undefined reference" "$tmp/unresolved.err"; then
    echo "unresolved-symbol check failed for the wrong reason" >&2
    cat "$tmp/unresolved.err" >&2
    exit 1
fi

echo "iedoom_link tests passed"
