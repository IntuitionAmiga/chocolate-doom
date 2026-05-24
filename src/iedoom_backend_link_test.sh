#!/bin/sh
set -eu

tmp="${TMPDIR:-/tmp}/iedoom-backend-link-test"
img="$tmp/iedoom.ie86"
elf="$tmp/iedoom.elf"

rm -rf "$tmp"
mkdir -p "$tmp"

sh src/iedoom_build.sh "$img" >/dev/null

if nm --undefined-only "$elf" | grep .; then
    echo "backend smoke image has unresolved symbols" >&2
    exit 1
fi

if [ ! -s "$img" ]; then
    echo "backend smoke image did not emit .ie86 binary" >&2
    exit 1
fi

echo "iedoom_backend_link tests passed"
