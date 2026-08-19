# IMGENGINE Native Benchmarking

**Status:** implemented baseline harness; performance gates are planned after repeated measurements on a controlled Linux runner.

## Measurement Contract

`bench_lat` is the canonical latency benchmark. It reports separately prepared render, decode, encode, and cold decode-plus-encode paths. The prepared render metric excludes final encoding and output I/O; it is the only metric relevant to the RFC render-only target. It must never be presented as SaaS end-to-end latency.

The committed `imgengine/photo.jpg` fixture is the baseline input. Every result records its SHA-256, byte length, preset, warm-up count, iteration count, Git revision, compiler, CMake version, kernel, and CPU topology. Results from different machines are informational only and must not be compared as a regression.

## Run

On the supported Linux environment:

```bash
bash imgengine/scripts/production_benchmark.sh
```

The command creates `imgengine/build/benchmark-results/<UTC timestamp>/` containing environment metadata and human-readable latency and decoder outputs. Keep baseline evidence outside Git, such as CI artifacts or a controlled performance-results store.

Use a fixed CPU governor, an otherwise idle runner, the same container/image, and at least five independent runs before proposing a threshold. Report the median of run-level p95 values, plus the worst p99. Do not run performance gates on shared GitHub-hosted runners.

## CI Policy

PR CI builds benchmark binaries to prevent bit rot, but does not enforce latency thresholds on variable hardware. A dedicated, pinned Linux performance runner publishes evidence and compares only equivalent fixture, build flags, CPU model, microcode/kernel, and compiler combinations. Any optimization change includes before/after evidence, output-correctness regression, and fallback-path coverage.

The regular Linux CI sanitizer job is a correctness prerequisite for performance work. Do not accept a benchmark improvement that relies on undefined behavior, memory leaks, or sanitizer suppression.

The manual `imgengine-native-performance` GitHub Actions workflow runs only on a self-hosted runner labelled `linux`, `x64`, and `imgengine-perf`; it uploads raw evidence for 90 days. Never add `ubuntu-latest` as a fallback runner.

## Performance Runner Standard

1. Provision Ubuntu with the native toolchain from [Native Development](NATIVE_DEVELOPMENT.md), a fixed `performance` CPU governor, and no concurrent benchmark workloads. `check_benchmark_host.sh --strict` enforces this before workflow execution.
2. Register it as a repository runner with the `imgengine-perf` label and restrict workflow-dispatch permission to performance owners.
3. Record CPU model, microcode, kernel, governor, turbo/boost policy, compiler, CMake, fixture hash, Git revision, and Git cleanliness for every run; the harness writes this to `environment.txt` and `preflight.txt`.
4. Trigger five independent workflow runs for the same revision. Retain the artifacts and record the median run-level p95 plus worst p99 as the proposed baseline.
5. Review output-correctness and scalar/SIMD regression evidence before accepting any performance improvement. Add a threshold only after this baseline is stable.

## Baseline Analysis

Download the five workflow artifacts and pass their extracted evidence directories to the analyzer:

```bash
python3 imgengine/scripts/analyze_benchmark_baseline.py run-1 run-2 run-3 run-4 run-5 \
  --output proposed-baseline.json
```

The analyzer rejects dirty worktrees, missing strict-governor evidence, and any mismatch in revision, fixture, preset, sample counts, compiler, CMake version, or CPU model. Its output reports `median_run_p95_ms` and `worst_run_p99_ms` for the prepared-render stage. Review and approve this JSON as a baseline record before encoding an enforceable regression threshold.

## Current Boundaries

- The `<2 ms` target applies only to the explicitly defined 4K prepared render benchmark, not the default photo fixture or cold path.
- Decoder timing is diagnostic; it includes decoder-library behavior and is not a SIMD-kernel claim.
- `perf.sh` remains a Linux diagnostic tool for CPU profiling after a reproducible baseline identifies a regression.
