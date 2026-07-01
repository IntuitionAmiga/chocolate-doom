#!/bin/sh
set -eu

out="${1:-build/iedoom.ie68}"
tmp="${TMPDIR:-/tmp}/iedoom-m68k-build"
elf="${out%.ie68}.elf"
config_dir="${IEDOOM_CONFIG_DIR:-/tmp/choc-ie-config}"

find_tool()
{
    for tool in "$@"; do
        if command -v "$tool" >/dev/null 2>&1; then
            command -v "$tool"
            return 0
        fi
    done
    return 1
}

if [ -z "${M68K_CC:-}" ]; then
    M68K_CC=$(find_tool \
        m68k-elf-gcc \
        m68k-atari-mint-gcc \
        m68k-linux-gnu-gcc-13 \
        m68k-linux-gnu-gcc \
        m68k-suse-linux-gcc || true)
fi

if [ -n "${M68K_CC:-}" ] && [ -z "${M68K_OBJCOPY:-}" ]; then
    case "$(basename "$M68K_CC")" in
        m68k-elf-gcc) M68K_OBJCOPY=$(find_tool m68k-elf-objcopy || true) ;;
        m68k-atari-mint-gcc) M68K_OBJCOPY=$(find_tool m68k-atari-mint-objcopy || true) ;;
        m68k-linux-gnu-gcc-13|m68k-linux-gnu-gcc) M68K_OBJCOPY=$(find_tool m68k-linux-gnu-objcopy || true) ;;
        m68k-suse-linux-gcc) M68K_OBJCOPY=$(find_tool m68k-suse-linux-objcopy || true) ;;
    esac
fi

if [ -n "${M68K_CC:-}" ] && [ -z "${M68K_NM:-}" ]; then
    case "$(basename "$M68K_CC")" in
        m68k-elf-gcc) M68K_NM=$(find_tool m68k-elf-nm || true) ;;
        m68k-atari-mint-gcc) M68K_NM=$(find_tool m68k-atari-mint-nm || true) ;;
        m68k-linux-gnu-gcc-13|m68k-linux-gnu-gcc) M68K_NM=$(find_tool m68k-linux-gnu-nm || true) ;;
        m68k-suse-linux-gcc) M68K_NM=$(find_tool m68k-suse-linux-nm || true) ;;
    esac
fi

if [ -n "${M68K_CC:-}" ] && [ -z "${M68K_OBJDUMP:-}" ]; then
    case "$(basename "$M68K_CC")" in
        m68k-elf-gcc) M68K_OBJDUMP=$(find_tool m68k-elf-objdump || true) ;;
        m68k-atari-mint-gcc) M68K_OBJDUMP=$(find_tool m68k-atari-mint-objdump || true) ;;
        m68k-linux-gnu-gcc-13|m68k-linux-gnu-gcc) M68K_OBJDUMP=$(find_tool m68k-linux-gnu-objdump || true) ;;
        m68k-suse-linux-gcc) M68K_OBJDUMP=$(find_tool m68k-suse-linux-objdump || true) ;;
    esac
fi

if [ -z "${M68K_CC:-}" ] || [ -z "${M68K_OBJCOPY:-}" ] || [ -z "${M68K_NM:-}" ] || [ -z "${M68K_OBJDUMP:-}" ]; then
    echo "missing M68K cross tools; set M68K_CC, M68K_OBJCOPY, M68K_NM, and M68K_OBJDUMP" >&2
    exit 1
fi

cpu_flags="-m68020 -mtune=68020 -m68881 -mhard-float"
opt_flags="-Ofast -ffreestanding -nostdlib -fno-builtin -fno-pic -fno-pie -fno-stack-protector"
cflags="$cpu_flags $opt_flags -fno-asynchronous-unwind-tables \
    -DINTUITION_ENGINE -DIEDOOM_GUEST -DIEDOOM_M68K -DDISABLE_SDL2MIXER -DDISABLE_SDL2NET \
    -I$config_dir -Isrc/iedoom/include -Isrc -I."

rm -rf "$tmp"
mkdir -p "$tmp" "$(dirname "$out")" "$(dirname "$elf")" "$config_dir"
if [ ! -f "$config_dir/config.h" ]; then
    cat > "$config_dir/config.h" <<'CONFIG_EOF'
#ifndef IEDOOM_CONFIG_H
#define IEDOOM_CONFIG_H
#define PACKAGE_NAME "Chocolate Doom"
#define PACKAGE_TARNAME "chocolate-doom"
#define PACKAGE_VERSION "iedoom"
#define PACKAGE_STRING "Chocolate Doom IEDoom"
#define PROGRAM_PREFIX "chocolate-doom"
#define HAVE_DECL_STRCASECMP 1
#define HAVE_DECL_STRNCASECMP 1
#endif
CONFIG_EOF
fi

link_script="src/iedoom_m68k_link.ld"
case "$(basename "$M68K_CC")" in
    m68k-atari-mint-gcc)
        link_script="$tmp/iedoom_m68k_mint_link.ld"
        sed \
            -e 's/OUTPUT_FORMAT("elf32-m68k")/OUTPUT_FORMAT(a.out-zero-big)/' \
            -e '/OUTPUT_ARCH/d' \
            -e '/PHDRS/,/}/d' \
            -e 's/ :text//g' \
            -e 's/ :data//g' \
            src/iedoom_m68k_link.ld > "$link_script"
        ;;
esac
ldflags="$cpu_flags -nostdlib -Wl,-T,$link_script"
libgcc=$("$M68K_CC" $cpu_flags -print-libgcc-file-name)

"$M68K_CC" $cflags -c src/iedoom_m68k_start.S -o "$tmp/iedoom_m68k_start.o"
"$M68K_CC" $cflags -c src/iedoom_runtime.c -o "$tmp/iedoom_runtime.o"
"$M68K_CC" $cflags -c src/iedoom_main.c -o "$tmp/iedoom_main.o"
"$M68K_CC" $cflags -c src/i_timer.c -o "$tmp/i_timer.o"
"$M68K_CC" $cflags -c src/i_intuition.c -o "$tmp/i_intuition.o"
"$M68K_CC" $cflags -c src/m_argv.c -o "$tmp/m_argv.o"
"$M68K_CC" $cflags -c src/m_misc.c -o "$tmp/m_misc.o"
"$M68K_CC" $cflags -c src/d_iwad.c -o "$tmp/d_iwad.o"
"$M68K_CC" $cflags -c src/w_file.c -o "$tmp/w_file.o"
"$M68K_CC" $cflags -c src/w_file_intuition.c -o "$tmp/w_file_intuition.o"
"$M68K_CC" $cflags -c src/w_wad.c -o "$tmp/w_wad.o"
"$M68K_CC" $cflags -c src/w_main.c -o "$tmp/w_main.o"
"$M68K_CC" $cflags -c src/z_zone.c -o "$tmp/z_zone.o"
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
    "$M68K_CC" $cflags -Isrc/doom -c "$source" \
        -o "$tmp/$(basename "$source" .c).o"
done
"$M68K_CC" $cflags -c src/iedoom_link_stubs.c -o "$tmp/iedoom_link_stubs.o"
"$M68K_CC" $cflags -c src/iedoom_backend_stubs.c -o "$tmp/iedoom_backend_stubs.o"

objects=
for object in "$tmp/"*.o; do
    case "$(basename "$object")" in
        iedoom_m68k_start.o) continue ;;
    esac
    objects="$objects $object"
done

"$M68K_CC" $ldflags -o "$elf" "$tmp/iedoom_m68k_start.o" $objects "$libgcc"

if "$M68K_NM" --undefined-only "$elf" | grep .; then
	echo "iedoom m68k image has unresolved symbols or invalid relocations" >&2
	exit 1
fi

if "$M68K_OBJDUMP" -d "$elf" | grep -Eiq '\<(call|cli|hlt)\>'; then
    echo "iedoom m68k image contains x86-only runtime instructions" >&2
	exit 1
fi

case "$(basename "$M68K_CC")" in
    m68k-atari-mint-gcc)
        "$M68K_CC" $ldflags -Wl,--oformat,binary \
            -o "$out" "$tmp/iedoom_m68k_start.o" $objects "$libgcc"
        ;;
    *)
        "$M68K_OBJCOPY" -O binary "$elf" "$out"
        ;;
esac
echo "$out"
