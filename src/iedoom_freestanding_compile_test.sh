#!/bin/sh
set -eu

tmp="${TMPDIR:-/tmp}/iedoom-freestanding-compile-test"
rm -rf "$tmp"
mkdir -p "$tmp"

cflags="-m32 -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -DINTUITION_ENGINE -DDISABLE_SDL2MIXER \
    -I/tmp/choc-ie-config -Isrc/iedoom/include -Isrc -I."

cc $cflags -c src/i_timer.c -o "$tmp/i_timer.o"
cc $cflags -c src/i_intuition.c -o "$tmp/i_intuition.o"
cc $cflags -c src/iedoom_main.c -o "$tmp/iedoom_main.o"
cc $cflags -c src/m_argv.c -o "$tmp/m_argv.o"
cc $cflags -c src/m_misc.c -o "$tmp/m_misc.o"
cc $cflags -c src/d_iwad.c -o "$tmp/d_iwad.o"

echo "iedoom_freestanding_compile tests passed"
