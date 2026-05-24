#!/bin/sh
set -eu

out="${1:-build/iedoom.ie86}"
sh src/iedoom_build.sh "$out"
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

echo "iedoom_build tests passed"
