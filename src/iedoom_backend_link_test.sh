#!/bin/sh
set -eu

tmp="${TMPDIR:-/tmp}/iedoom-backend-link-test"
elf="$tmp/iedoom-backend.elf"
libgcc=$(cc -m32 -print-libgcc-file-name)

rm -rf "$tmp"
mkdir -p "$tmp"

cflags="-m32 -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -DINTUITION_ENGINE -DDISABLE_SDL2MIXER \
    -I/tmp/choc-ie-config -Isrc/iedoom/include -Isrc -I."

nasm -f elf32 -I ../IntuitionEngine/sdk/include/ \
    -o "$tmp/iedoom_boot.o" src/iedoom_boot.asm
cc $cflags -c src/iedoom_runtime.c -o "$tmp/iedoom_runtime.o"
cc $cflags -c src/iedoom_main.c -o "$tmp/iedoom_main.o"
cc $cflags -c src/i_timer.c -o "$tmp/i_timer.o"
cc $cflags -c src/i_intuition.c -o "$tmp/i_intuition.o"
cc $cflags -c src/m_argv.c -o "$tmp/m_argv.o"
cc $cflags -c src/m_misc.c -o "$tmp/m_misc.o"
cc $cflags -c src/d_iwad.c -o "$tmp/d_iwad.o"
cc $cflags -c src/iedoom_link_stubs.c -o "$tmp/iedoom_link_stubs.o"
cc $cflags -c src/iedoom_backend_stubs.c -o "$tmp/iedoom_backend_stubs.o"

ld -m elf_i386 -nostdlib -T src/iedoom_link.ld \
    -o "$elf" "$tmp/iedoom_boot.o" "$tmp/iedoom_runtime.o" \
    "$tmp/iedoom_main.o" "$tmp/i_timer.o" "$tmp/i_intuition.o" \
    "$tmp/m_argv.o" "$tmp/m_misc.o" "$tmp/d_iwad.o" \
    "$tmp/iedoom_link_stubs.o" "$tmp/iedoom_backend_stubs.o" "$libgcc"

if nm --undefined-only "$elf" | grep .; then
    echo "backend smoke image has unresolved symbols" >&2
    exit 1
fi

echo "iedoom_backend_link tests passed"
