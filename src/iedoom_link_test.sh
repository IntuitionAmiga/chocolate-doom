#!/bin/sh
set -eu

tmp="${TMPDIR:-/tmp}/iedoom-link-test"
elf="$tmp/iedoom.elf"
bin="$tmp/iedoom.ie86"

rm -rf "$tmp"
mkdir -p "$tmp"

nasm -f elf32 -I ../IntuitionEngine/sdk/include/ \
    -o "$tmp/iedoom_boot.o" src/iedoom_boot.asm
cc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
    -fno-asynchronous-unwind-tables -c src/iedoom_runtime.c \
    -o "$tmp/iedoom_runtime.o"
ld -m elf_i386 -nostdlib -T src/iedoom_link.ld \
    -o "$elf" "$tmp/iedoom_boot.o" "$tmp/iedoom_runtime.o"
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
if [ -z "$bss_start" ] || [ -z "$bss_end" ] || [ -z "$bss_probe" ]; then
    echo "linked image is missing bss symbols" >&2
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

entry_byte=$(od -An -tx1 -j4096 -N1 "$bin" | tr -d ' \n')
if [ "$entry_byte" != "fa" ]; then
    echo "iedoom_entry does not begin with cli: $entry_byte" >&2
    exit 1
fi

echo "iedoom_link tests passed"
