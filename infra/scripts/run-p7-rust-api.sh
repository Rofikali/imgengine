#!/usr/bin/env bash
set -euo pipefail

source_dir=/workspace/imgengine/imgengine
native_build=/workspace/native-build

cmake -S "$source_dir" -B "$native_build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DIMGENGINE_BENCH=OFF \
  -DIMGENGINE_LTO=OFF
cmake --build "$native_build" --parallel "$(nproc)"

export LD_LIBRARY_PATH="$native_build${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export RUSTFLAGS="-L native=$native_build${RUSTFLAGS:+ $RUSTFLAGS}"
exec cargo run --manifest-path "$source_dir/rust/imgengine-api/Cargo.toml" --bin imgengine-api-server
