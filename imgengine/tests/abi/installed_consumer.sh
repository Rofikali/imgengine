#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
source_dir="$2"
sanitize_flags="${3:-}"
stage_dir="$(mktemp -d)"
trap 'rm -rf "$stage_dir"' EXIT

DESTDIR="$stage_dir" cmake --install "$build_dir" --prefix /usr >/dev/null
cc -std=c11 -Wall -Wextra -Werror $sanitize_flags -I"$stage_dir/usr/include" \
    "$source_dir/tests/abi/test_public_consumer.c" -L"$stage_dir/usr/lib" -limgengine \
    -Wl,-rpath,"$stage_dir/usr/lib" -pthread $sanitize_flags -o "$stage_dir/consumer"
"$stage_dir/consumer"
