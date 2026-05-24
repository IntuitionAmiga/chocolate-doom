#!/bin/sh
set -eu

out="${TMPDIR:-/tmp}/iedoom_start.ie86"
nasm -f bin -I src/ -I ../IntuitionEngine/sdk/include/ \
    -o "$out" src/iedoom_start.asm

size=$(wc -c < "$out")
if [ "$size" -lt 4096 ]; then
    echo "iedoom_start image is smaller than PROGRAM_START" >&2
    exit 1
fi

prefix=$(od -An -tx1 -N10 "$out" | tr -d ' \n')
if [ "$prefix" != "bc0000ff00e9f60f0000" ]; then
    echo "unexpected reset trampoline bytes: $prefix" >&2
    exit 1
fi

entry_byte=$(od -An -tx1 -j4096 -N1 "$out" | tr -d ' \n')
if [ "$entry_byte" != "f4" ]; then
    echo "PROGRAM_START placeholder is not hlt: $entry_byte" >&2
    exit 1
fi

echo "iedoom_start tests passed"
