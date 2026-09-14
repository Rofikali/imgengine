# Scheduler Characterization Decision — 2026-09-14

## Status

**Decision: A — keep scheduler ownership in C.** Rust remains a bounded
admission/control layer only. This is a decision gate, not a scheduler
migration.

## Reproducible Run

- Commit under test: `ed35e639e2fa573607daeaff8515d092f72611c6`
- Image: `imgengine-verify:ubuntu24-format-check` (Ubuntu 24.04.4)
- Toolchain: GCC 13.3.0, CMake 3.28.3, Rust/Cargo 1.75.0
- Host: WSL2 Linux 5.15.167.4, Intel Core i5-1235U, 12 logical CPUs, 3.75 GiB memory
- Command: `bash scripts/run_scheduler_characterization.sh --iterations 30 --warmup 5 --concurrency 4 --queue-capacity 4`
- Fixture inputs: generated 320x240 JPEG/PNG, 1600x1200 JPEG, 2048x1536
  JPEG, 640x427 progressive JPEG, and a deliberately truncated JPEG.
- Evidence was written to the ignored local directory
  `imgengine/build/scheduler-characterization-results/final-20260914`.

The Docker bind mount cannot resolve Windows Git worktree pointer files. The
runner therefore receives the commit through `IMGENGINE_GIT_REVISION`; this is
an environment limitation, not a product-code limitation.

## Results

P50/P95 are successful-request wall latency. `C` is direct public ABI use;
`Rust` is bounded Rust admission followed by the same C execution path.

| Workload | C P50/P95 ms | Rust P50/P95 ms | C ops/s | Rust ops/s | Result |
|---|---:|---:|---:|---:|---|
| small JPEG sequential | 0.683 / 0.810 | 1.181 / 10.432 | 735.60 | 400.59 | threshold fail |
| small PNG sequential | 1.493 / 21.557 | 1.649 / 13.048 | 345.35 | 298.28 | P50 pass; P95 pass |
| representative JPEG sequential | 43.472 / 66.513 | 48.109 / 76.423 | 21.18 | 19.83 | threshold pass |
| large JPEG sequential | 80.248 / 104.260 | 79.809 / 107.683 | 12.32 | 12.31 | threshold pass |
| progressive JPEG sequential | 7.921 / 28.663 | 13.030 / 31.392 | 66.97 | 59.57 | threshold fail |
| small JPEG concurrent | 1.609 / 2.571 | 3.313 / 22.185 | 228.85 | 451.10 | threshold fail |
| representative JPEG concurrent | 110.069 / 159.927 | 118.131 / 225.908 | 24.43 | 19.13 | threshold fail |
| large JPEG concurrent | 179.978 / 291.745 | 180.186 / 341.115 | 13.38 | 12.15 | threshold fail |

All valid-image runs completed successfully. The truncated JPEG produced 30
expected failures through each path and no output. Rust saturation at queue
capacity one accepted 7 of 30 concurrent attempts, rejected 23 as overload,
and reported maximum queue depth one. Every Rust run reported an empty
workspace after orderly shutdown.

## Threshold Assessment

The provisional contract permits at most 15% sequential Rust wall-latency
overhead and 25% concurrent P95 overhead with capacity at least concurrency.
The full run meets this for representative and large sequential JPEGs, but not
for small JPEG, progressive JPEG, or any concurrent workload. The thresholds
therefore do **not** admit scheduler migration.

This is not a production defect: the benchmark intentionally measures Rust
owned-input copying, channel admission, and one-engine serialization. The
numbers are host-specific and are not capacity claims.

## Lifecycle and Safety Evidence

`imgengine-supervisor` tests exercise bounded overload response, input limits,
malformed input, dropped response handles, orderly shutdown/drain, rejection
after shutdown, cleanup after success/failure, and repeated start/stop cycles.
No mid-operation cancellation is added; accepted work drains according to ABI
v1. The native C scheduler and security/sandbox implementation are unchanged.

## Decision Answers

1. **Is Rust admission overhead acceptable?** Not against every provisional
   threshold; it is acceptable only for the larger sequential fixtures here.
2. **Does the C scheduler meet these workloads?** It completed all valid
   requests and rejected malformed input safely in this limited host run.
3. **Should Rust own scheduling now?** No.
4. **What should move first if later justified?** Only HTTP-facing admission,
   overload mapping, and lifecycle telemetry—not native execution scheduling.
5. **What remains in C?** Engine serialization, execution, native resources,
   sandbox lifecycle, decoding, rendering, and output ownership.
6. **What evidence is missing?** Three stable pinned-host runs, a licensed
   project-owned real-camera CI fixture, and production-like load/RSS sampling.

The next architecture step is to retain this contract and collect the missing
evidence; do not migrate scheduler ownership before a separate review.
