# P7.4-A SIMD-001 Evidence

## Objective and accountability

- Owner: **OWNER-C**; security review: **OWNER-SEC**; verification:
  **OWNER-OPS**; scope authority: **OWNER-ARCH**.
- Change class: **MINIMAL CORRECTNESS FIX**, authorized after executable
  divergence evidence.
- Baseline: `90cedaf` plus preserved, verified, uncommitted P7.2.8 work.
- Entry criteria: P7.3 review complete, scope frozen, no ABI/API/scheduler
  change authorized, Ubuntu baseline known PASS.

## Source and reachability trace

| Path | Source evidence | Reachability |
| --- | --- | --- |
| Scalar grayscale | `src/arch/x86_64/scalar/color_scalar.c` | active native transform capability |
| AVX2 grayscale | `src/arch/x86_64/avx2/color_avx2.c` | selected through `hardware_registry.c` on AVX2/AVX512 builds |
| Fused scalar / AVX2 | `pipeline_fuse_exec_scalar.c`, `pipeline_fuse_exec_avx2.c`, `pipeline_fuse_select.c` | plugin/fused path; not proven exposed by ABI v1 raw JPEG service path |
| Verified SaaS/native API | `imgengine_v1.c` -> `img_api_process_raw` -> decoder/encoder | does not construct grayscale/fused transform pipeline |

The standalone scalar formula is per RGB pixel:
`(R*77 + G*150 + B*29) >> 8`, replicated into all three channels. The AVX2
vector loop in `color_avx2.c` applies coefficient `77` to every byte. The fused
scalar and AVX2 implementations likewise process individual bytes with `77`.
These are not equivalent semantics.

## Independent oracle and fixture matrix

`src/tests/test_resize_kernel_equivalence.c` contains a test-only luma oracle;
it does not call a production grayscale or fused helper. Fixtures repeat explicit
black, white, neutral gray, red, green, blue, and mixed RGB triples and cover:

| Category | Cases |
| --- | --- |
| minimum / multiple rows | 1x1; 10x2, 11x3, 37x4 |
| alignment | aligned and deliberately +1 unaligned data |
| stride / tails | padding 0, 2, 5, 7; widths below and above 32-byte vector span |
| brightness | 0, 17, 200 |
| implementations | standalone scalar, standalone AVX2, fused scalar, fused AVX2 |

## Actual evidence

Focused command:

```text
cmake --build build-p7 --target test_resize_kernel_equivalence -j2
./build-p7/test_resize_kernel_equivalence
```

Observed:

```text
tail scalar=1 avx=1 fused-scalar=0 fused-avx=0
simd scalar=1 avx=0 fused-scalar=0 fused-avx=0
padded fused-scalar=0 fused-avx=0; brightness fused-scalar=0 fused-avx=0
SIMD-001 divergence reproduced: AVX2/fused paths are not RGB-luma equivalent
```

The 10-pixel tail does not enter the 32-byte standalone AVX2 loop, explaining
its scalar-equivalent result. The 11-pixel case does enter that loop and proves
the standalone AVX2 mismatch. Fused scalar and fused AVX2 mismatch on the
tested paths. This is executable divergence evidence, not a test failure being
treated as success.

Focused sanitizer command:

```text
cmake --build build-p7-asan --target test_resize_kernel_equivalence -j2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ./build-p7-asan/test_resize_kernel_equivalence
```

Result: PASS; no ASAN, UBSAN, or leak finding.

## Decision and exit status

The original evidence-only result was **REAL CORRECTNESS DIVERGENCE — FIX
REQUIRED**. Separate authorization then changed this item to the
`MINIMAL CORRECTNESS FIX` class.

The correction preserves AVX2/fused entry points and dispatch, but routes them
to the corrected scalar RGB-luma implementation until a separately verified
RGB-deinterleaving SIMD kernel exists. `pipeline_fuse_exec_scalar.c` now applies
RGB luma per pixel before brightness, and `pipeline_fuse_exec_avx2.c` delegates
to it. This is deliberately a correctness fallback, not a SIMD optimization;
any performance effect is unmeasured and deferred.

After correction, the same oracle matrix reports scalar, AVX2, fused scalar,
and fused AVX2 all equal across tail, vector-crossing, stride, unaligned, and
brightness cases. Focused ASAN/UBSAN/leak evidence also passes. ABI/API,
scheduler, dependency, and product behavior are unchanged.

## Principal re-audit

| ID | Requirement | Evidence | Result | Remaining Risk |
| --- | --- | --- | --- | --- |
| SIMD-001-R01 | Independent RGB-luma oracle | test-only reference in `test_resize_kernel_equivalence.c` | PASS | none for tested RGB semantics |
| SIMD-001-R02 | Scalar / AVX2 equivalence | focused matrix after correction | PASS | AVX2 entry delegates to scalar pending a future separately optimized kernel |
| SIMD-001-R03 | Fused equivalence | focused scalar/fused-AVX2 matrix | PASS | none for tested grayscale/brightness contract |
| SIMD-001-R04 | Alignment, stride, tails | +1 unaligned, padding 0/2/5/7, 1x1 through vector-crossing widths | PASS | broader randomized transform corpus deferred |
| SIMD-001-R05 | Dispatch / fallback | AVX2 public entry and fused AVX2 entry preserve dispatch but call scalar-equivalent implementation | PASS | performance impact deliberately unmeasured |
| SIMD-001-R06 | ASAN / UBSAN / leak | focused corrected-tree sanitizer command | PASS | none observed |
| SIMD-001-R07 | Native regressions / full Ubuntu verifier | fresh `IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh` | PASS | bounded fuzzing remains smoke evidence, not proof of absence |
| SIMD-001-R08 | ABI / API compatibility | public ABI unchanged; verifier completed ABI consumers, Rust FFI/API and real-image checks | PASS | no grayscale transform exposure is claimed |
| SIMD-001-R09 | Performance impact | no benchmark run; fallback selected deliberately | NOT APPLICABLE to correctness closure | future optimized RGB-deinterleaving kernel needs performance/equivalence gate |

The fresh Ubuntu 24.04 container exited `0` and reported `Linux verification
passed: /workspace/build-linux-verify`; it reached format-check, normal and
ASAN CTest/regressions, ABI/FFI/supervisor/API, real-image checks, sandbox, and
both 5,000-run fuzz targets. The prior Cargo-stalled run was discarded as a
mixed-tree/incomplete attempt; it does not contribute to this conclusion.

**Final decision: SIMD-001 CLOSED.** The demonstrated correctness risk was
removed with a small scalar-equivalent fallback behind unchanged SIMD entry
points. This adds no ABI/API/dependency/scheduler complexity, but intentionally
defers SIMD performance restoration to a separately approved and benchmarked
optimization task.
