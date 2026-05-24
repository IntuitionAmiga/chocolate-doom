#!/bin/sh
set -eu

obj="${TMPDIR:-/tmp}/iedoom_runtime_symbols.o"
cc -m32 -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -DIEDOOM_GUEST -c src/iedoom_runtime.c -o "$obj"

for symbol in memset memcpy memmove memcmp strlen strcmp strncmp strcpy strncpy \
    strcasecmp strncasecmp atoi atof abs rand srand malloc calloc realloc free; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

for symbol in isspace isdigit isalpha isalnum isprint toupper tolower exit; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

for symbol in printf fprintf vprintf vfprintf snprintf vsnprintf sscanf fscanf \
    puts putchar fopen fclose fread fwrite feof fgetc ungetc fseek ftell remove \
    rename getenv stat mkdir strdup strchr strrchr strstr SDL_GetPrefPath \
    SDL_free SDL_qsort setlocale localeconv time fabs; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

for symbol in __udivdi3 __umoddi3 __divdi3; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

echo "iedoom_runtime_symbols tests passed"
