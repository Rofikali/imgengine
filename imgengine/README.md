# IMGENGINE native C17 core

**Scope:** the active native C image-processing library, public versioned ABI, internal CLI/layout/scheduler, SIMD, allocators, security, observability, tests and packaging. Application HTTP/auth/billing is Rust under [backend/](../backend/); the current frontend is [Nuxt](../imgengine-saas/frontend/). **This directory is not a new Python/FastAPI/Redis/PostgreSQL service.**

**Start a review:** [native AGENTS](AGENTS.md), [principal A–Z source/LLD/DSA/security audit](../docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [public ABI contracts](../docs/ABI.md), [C/Rust evidence matrix](../docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md), [native benchmark rules](../docs/NATIVE_BENCHMARKING.md), [root next-action board](../next.md) and [Astra handoff](../docs/ASTRA_ENGINEERING_START_HERE.md).

## Current architecture

~~~text
Versioned libimgengine.so C ABI + opaque handle
  -> decoder vtable (JPEG TurboJPEG, PNG/stb)
  -> validated private layout + normalized crop
  -> native scheduler/worker task
  -> slab/arena-backed canvas and scalar/SIMD image algorithms
  -> bounded JPEG encoder and caller-owned output
~~~

The CLI and historical internal APIs have additional code paths. **Only installed headers under include/imgengine/api and symbols in cmake/libimgengine.map are stable public C SDK declarations.** CMake builds an internal static library but does not provide a separately installed public libimgengine.a SDK.

## Source ownership map

| Native subsystem | Directory / contract |
| --- | --- |
| Public ABI and status/ownership | include/imgengine/api/, src/api/imgengine_v1.c and imgengine_v2_layout.c, imgengine_v2_sheet.c |
| JPEG/PNG decode, JPEG/PDF internals | src/io/decoder/, src/io/encoder/ |
| Geometry, canvas, FIT/FILL/crop and resize | src/pipeline/ |
| Scalar, AVX2, AVX512 and runtime capability selection | src/arch/ and src/pipeline/jump_table* |
| Slab, arena, poison and NUMA | src/memory/ |
| Native worker/queues/scheduler | src/runtime/ |
| Validation/seccomp/fuzz | src/security/ |
| Internal metrics/logging/profiling | src/observability/ |
| Tests/ABI, integration and benchmarks | src/tests/, tests/, scripts/verify_linux.sh, CMakeLists.txt |

## First engineering review priority

Start the **C-A0 read-only concurrency audit** in the [principal source report](../docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md): the single-consumer ring's relationship to multiple work stealers, and native metrics synchronization/bounds. Do not assert a confirmed exploit, rewrite scheduling or relax CI until source reachability, a deterministic reproducer and the narrowest safe fix are agreed.

Preserve strict shipping C17, C11 consumer/compatibility checks, stable ABI ownership, test oracles and security gates. **Speed must be measured** on controlled hardware, separately for tiny ns-scale functions, hot image kernels and complete decode/render/encode/API requests. A fast microbenchmark is not an image-service SLO.

For a docs-only task, verify claims and Markdown links; do **not** manually run native C sanitizer/fuzz/complete toolchains. Required GitHub checks and later release gates are never waived. Do not merge, deploy or change hosted services without explicit owner approval.
