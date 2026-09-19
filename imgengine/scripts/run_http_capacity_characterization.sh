#!/usr/bin/env bash
set -euo pipefail

# P6.5: Docker/WSL2 loopback characterization. This does not make a production
# capacity claim and deliberately preserves the P5/P6 provisional configuration.
source_dir="${IMGENGINE_SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
build_dir="${IMGENGINE_HTTP_CHARACTERIZATION_BUILD:-${source_dir}/build-http-characterization}"
results_dir="${IMGENGINE_HTTP_CHARACTERIZATION_RESULTS:-${build_dir}/results}"
# Do not reuse the verifier cache: its absolute source path is intentionally
# different when this script is run with the repository root mounted in Docker.
normal_dir="${IMGENGINE_HTTP_CHARACTERIZATION_NATIVE:-${build_dir}/native-normal}"
mkdir -p "$build_dir/fixtures" "$results_dir" "$build_dir/workspaces"

cmake -S "$source_dir" -B "$normal_dir" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DIMGENGINE_BENCH=OFF -DIMGENGINE_LTO=OFF
cmake --build "$normal_dir" --parallel "${IMGENGINE_VERIFY_JOBS:-$(nproc)}"
# Strip encoder metadata and reuse the project-owned photo for the larger
# fixture. This avoids a random plasma generator making runs incomparable.
convert -size 160x120 gradient: -strip -quality 88 "$build_dir/fixtures/small.jpg"
convert -size 160x120 gradient: -strip "$build_dir/fixtures/small.png"
cp "$source_dir/photo.jpg" "$build_dir/fixtures/large.jpg"

report="$results_dir/http_capacity.json"
metadata="$results_dir/environment.txt"
{
  echo "label=CHARACTERIZATION — Docker/WSL2 loopback only; not production capacity"
  echo "git_commit=$(git -C "$source_dir/.." rev-parse HEAD 2>/dev/null || echo uncommitted-worktree)"
  echo "os=$(grep PRETTY_NAME /etc/os-release)"
  echo "kernel=$(uname -a)"
  echo "rust=$(rustc --version)"
  echo "cargo=$(cargo --version)"
  echo "cc=$(cc --version | head -n1)"
  echo "cmake=$(cmake --version | head -n1)"
  echo "ninja=$(ninja --version)"
  echo "cpu=$(grep -m1 'model name' /proc/cpuinfo || true)"
  echo "cpu_count=$(nproc)"
  echo "memory=$(grep MemTotal /proc/meminfo)"
  sha256sum "$build_dir/fixtures"/*
  echo "command=cargo run --release --bin http_capacity_characterization ..."
  echo "configuration=body=1048576 queue=1 rust_workers=1 deadline_seconds=10"
} > "$metadata"

# Ubuntu's minimal verifier image need not install the external GNU time
# binary, so use Bash's portable `time -p` instead. Its user/sys/real values
# are process accounting for the whole harness, not host utilization.
{ time -p env \
  LD_LIBRARY_PATH="$normal_dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
  RUSTFLAGS="-L native=$normal_dir" \
  cargo run --release --quiet --manifest-path "$source_dir/rust/imgengine-api/Cargo.toml" \
  --bin http_capacity_characterization -- \
  "$build_dir/workspaces" "$build_dir/fixtures/small.jpg" "$build_dir/fixtures/small.png" "$build_dir/fixtures/large.jpg" "$report"; } \
  2> "$results_dir/resources.txt"

echo "HTTP characterization evidence written to: $results_dir"
