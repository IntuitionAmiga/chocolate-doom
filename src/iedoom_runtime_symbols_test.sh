#!/bin/sh
set -eu

obj="${TMPDIR:-/tmp}/iedoom_runtime_symbols.o"
cc -m32 -ffreestanding -fno-builtin -fno-pic -fno-pie \
    -fno-stack-protector -fno-asynchronous-unwind-tables \
    -c src/iedoom_runtime.c -o "$obj"

for symbol in memset memcpy memmove memcmp strlen strcmp strncmp strcpy strncpy \
    strcasecmp strncasecmp atoi malloc free; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

for symbol in isspace isdigit isalpha isalnum toupper tolower exit; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

for symbol in printf fprintf vprintf vfprintf snprintf vsnprintf sscanf puts \
    putchar fopen fclose fread fwrite fseek ftell remove rename getenv stat \
    mkdir strdup strchr strrchr strstr SDL_qsort; do
    if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "$symbol"; then
        echo "missing freestanding runtime symbol: $symbol" >&2
        exit 1
    fi
done

if ! nm --defined-only "$obj" | awk '{ print $3 }' | grep -qx "__udivdi3"; then
    echo "missing freestanding runtime symbol: __udivdi3" >&2
    exit 1
fi

echo "iedoom_runtime_symbols tests passed"
