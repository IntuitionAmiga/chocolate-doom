#!/bin/sh
set -eu

out="${1:-build/iedoom.ie86}"
tmp="${TMPDIR:-/tmp}/iedoom-build"
elf="${out%.ie86}.elf"
cpu_flags="-m32 -march=i386 -mtune=i386 -m80387 -mhard-float -mfpmath=387 \
    -mno-sse -mno-sse2 -mno-mmx"
libgcc=$(cc $cpu_flags -print-libgcc-file-name)

rm -rf "$tmp"
mkdir -p "$tmp" "$(dirname "$out")" "$(dirname "$elf")"

cflags="$cpu_flags -Ofast \
    -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -DINTUITION_ENGINE -DIEDOOM_GUEST -DDISABLE_SDL2MIXER -DDISABLE_SDL2NET \
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
cc $cflags -c src/w_file.c -o "$tmp/w_file.o"
cc $cflags -c src/w_file_intuition.c -o "$tmp/w_file_intuition.o"
cc $cflags -c src/w_wad.c -o "$tmp/w_wad.o"
cc $cflags -c src/w_main.c -o "$tmp/w_main.o"
cc $cflags -c src/z_zone.c -o "$tmp/z_zone.o"
for source in \
    src/aes_prng.c \
    src/d_event.c \
    src/d_loop.c \
    src/d_mode.c \
    src/gusconf.c \
    src/i_sound.c \
    src/i_video.c \
    src/m_bbox.c \
    src/m_cheat.c \
    src/m_config.c \
    src/m_controls.c \
    src/m_fixed.c \
    src/memio.c \
    src/net_client.c \
    src/net_common.c \
    src/net_dedicated.c \
    src/net_io.c \
    src/net_loop.c \
    src/net_packet.c \
    src/net_petname.c \
    src/net_query.c \
    src/net_sdl.c \
    src/net_server.c \
    src/net_structrw.c \
    src/p_rejectpad.c \
    src/sha1.c \
    src/tables.c \
    src/v_diskicon.c \
    src/v_video.c \
    src/w_checksum.c \
    src/w_merge.c \
    src/doom/*.c
do
    case "$source" in
        */doom_icon.c) continue ;;
        */deh_*.c) continue ;;
    esac
    cc $cflags -Isrc/doom -c "$source" \
        -o "$tmp/$(basename "$source" .c).o"
done
cc $cflags -c src/iedoom_link_stubs.c -o "$tmp/iedoom_link_stubs.o"
cc $cflags -c src/iedoom_backend_stubs.c -o "$tmp/iedoom_backend_stubs.o"

ld -m elf_i386 -nostdlib -T src/iedoom_link.ld \
    -o "$elf" "$tmp/"*.o "$libgcc"

if nm --undefined-only "$elf" | grep .; then
    echo "iedoom image has unresolved symbols" >&2
    exit 1
fi

objcopy -O binary "$elf" "$out"
echo "$out"
