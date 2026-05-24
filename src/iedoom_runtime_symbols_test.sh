#!/bin/sh
set -eu

obj="${TMPDIR:-/tmp}/iedoom_runtime_symbols.o"
cc -m32 -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -c src/iedoom_runtime.c -o "$obj"

for symbol in memset memcpy memmove memcmp strlen strcmp strcpy strncpy; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

echo "iedoom_runtime_symbols tests passed"
