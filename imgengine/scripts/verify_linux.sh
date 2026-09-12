#!/usr/bin/env bash
set -euo pipefail

source_dir="${IMGENGINE_SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
build_root="${IMGENGINE_VERIFY_BUILD_ROOT:-${source_dir}/build-linux-verify}"
jobs="${IMGENGINE_VERIFY_JOBS:-$(nproc)}"
fuzz_runs="${IMGENGINE_FUZZ_RUNS:-5000}"

configure() {
    local build_dir="$1"
    shift
    cmake -S "$source_dir" -B "$build_dir" -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DIMGENGINE_BENCH=OFF \
        -DIMGENGINE_LTO=OFF \
        "$@"
}

run_ctest() {
    local build_dir="$1"
    cmake --build "$build_dir" --parallel "$jobs"
    ctest --test-dir "$build_dir" --output-on-failure
    cmake --build "$build_dir" --target regression_security regression_geometry regression_progressive
}

run_rust_ffi_smoke() {
    local build_dir="$1"
    LD_LIBRARY_PATH="$build_dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    RUSTFLAGS="-L native=$build_dir" \
        cargo run --quiet --manifest-path "$source_dir/rust/imgengine-ffi-smoke/Cargo.toml"
}

rm -rf "$build_root"
mkdir -p "$build_root"

configure "$build_root/normal"
run_ctest "$build_root/normal"
run_rust_ffi_smoke "$build_root/normal"

configure "$build_root/asan" -DIMGENGINE_SANITIZE=ON
ASAN_OPTIONS="detect_leaks=1:halt_on_error=1" \
UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
    run_ctest "$build_root/asan"

configure "$build_root/sandbox" -DIMGENGINE_SANDBOX=ON
cmake --build "$build_root/sandbox" --parallel "$jobs"
ctest --test-dir "$build_root/sandbox" -R '^sandbox_lifecycle$' --output-on-failure

CC=clang configure "$build_root/fuzz" \
    -DCMAKE_C_COMPILER=clang \
    -DIMGENGINE_FUZZ=ON \
    -DIMGENGINE_COVERAGE=ON
cmake --build "$build_root/fuzz" --target fuzz_input_validator fuzz_decoder --parallel "$jobs"
LLVM_PROFILE_FILE="$build_root/fuzz/input-validator-%p.profraw" \
    "$build_root/fuzz/fuzz_input_validator" -runs="$fuzz_runs" -max_len=4096
LLVM_PROFILE_FILE="$build_root/fuzz/decoder-%p.profraw" \
    "$build_root/fuzz/fuzz_decoder" -runs="$fuzz_runs" -max_len=1048576

echo "Linux verification passed: $build_root"
