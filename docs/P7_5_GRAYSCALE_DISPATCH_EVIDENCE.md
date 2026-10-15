# P7.5 Baseline Reconciliation and Grayscale Dispatch Correctness

## Scope and baseline

This record covers P75-001 only. It was investigated in the canonical
checkout on branch `codex/fix-saas-candidate-fixtures`, at
`3e369e8d67008eca779f79a62f0898a8649634cc`. No ABI v1, Rust, HTTP,
scheduler, or product-contract change is part of this work.

The supported grayscale semantic is the existing scalar RGB-luma operation:

```text
gray = (R * 77 + G * 150 + B * 29) >> 8
```

Each output RGB channel receives `gray`. This is truncating fixed-point
BT.601-style luma, not a byte-wise intensity scale.

## P75-001: live dispatch divergence

### Root cause and reachability

Engine startup calls `img_engine_init_compute()` before
`img_engine_init_plugins_and_io()`:

1. `img_hw_register_kernels()` selected `img_arch_grayscale_avx2` on AVX2.
2. Built-in plugin registration then overwrote `g_jump_table[OP_GRAYSCALE]`.
3. The historical plugin implementation applied `value * 77 >> 8` to every
   byte instead of RGB luma.

The live jump table is used by compiled pipelines and the internal RPC server.
The fused single-image and batch routes also used the same byte-wise
approximation. The ABI-v1 encoded-JPEG path was inspected but was not shown to
construct an `OP_GRAYSCALE` pipeline; therefore this record makes no claim of
current SaaS/HTTP reachability.

### Reproduction and correction

`test_resize_kernel_equivalence` now uses pixels that distinguish RGB luma from
the byte-wise approximation and independently checks:

- hardware-selected dispatch before plugin registration;
- the built-in plugin override after registration;
- a compiled pipeline;
- internal RPC dispatch;
- fused single-image execution; and
- fused batch execution.

The pre-correction hardware test failed with:

```text
hardware grayscale dispatch differs from RGB-luma oracle
```

The correction routes the AVX2 dispatch entry and fused routes to the existing
canonical scalar RGB-luma implementation. This deliberately preserves function
and dispatch boundaries while avoiding an unverified RGB-deinterleaving SIMD
implementation. A future optimized vector kernel requires a separate oracle
and performance gate.

## Verification

| Evidence | Command | Result |
| --- | --- | --- |
| Focused normal | `ctest --test-dir /tmp/p75-focused --output-on-failure -R '^resize_kernel_equivalence$'` | PASS |
| Focused ASAN/UBSAN/leak | `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir /tmp/p75-asan --output-on-failure -R '^resize_kernel_equivalence$'` | PASS |
| Authoritative Ubuntu 24.04 | `docker build -f imgengine/Dockerfile.p75-authoritative.tmp -t imgengine-p75-authoritative imgengine` | PASS |
| Diff whitespace | `git diff --check` | PASS |

The authoritative Dockerfile runs the unchanged `scripts/verify_linux.sh` with
`IMGENGINE_VERIFY_JOBS=2` and `IMGENGINE_FUZZ_RUNS=5000`. Its successful image
layer was created by that command, so normal and ASAN CTest/regressions,
Rust FFI/supervisor/API, real-image checks, sandbox, and both fuzz targets
completed successfully.

## Status

**P75-001: CLOSED / VERIFIED.**

Production readiness is not claimed.
