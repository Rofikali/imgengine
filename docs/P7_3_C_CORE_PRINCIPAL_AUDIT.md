# P7.3 C-Core Principal/Staff Audit

## Scope and result

This is a source/build/reachability audit of the C core at the verified P7.2.8
baseline, not a rewrite or production-readiness assessment. Detailed inventory,
decisions, and quarantine rules are companion documents.

## Findings

### Critical

None proven by this audit or the current Ubuntu verifier.

### High

1. **H-01 — input ownership has incompatible historical models.** Legacy
   `img_api_decode_input` releases mmap input; current raw processing instead
   calls the decoder vtable with a borrowed pointer. The distinction is correct
   in P7.2.8 but must be explicit at every future ABI/HTTP entry. Classification:
   HARDEN; no source change authorized in discovery.
2. **H-02 — SIMD-001 remains open.** Resize AVX2/scalar equivalence is tested,
   but grayscale/fused dispatch needs an independent scalar oracle over tails,
   alignment, and representative RGB inputs. Classification: HARDEN.

### Medium

1. **M-01 — scheduler wait is spin then `sched_yield`;** queue capacity and
   pointer lifetime rely on caller/worker protocol. Existing tests validate
   lifecycle paths, but starvation, saturation, and shutdown stress evidence is
   incomplete. Keep architecture; add evidence before changing it.
2. **M-02 — slab/arena safety is distributed.** Bounds checks and mutex use
   exist, but capacity, reuse, allocator-failure, and concurrent teardown
   coverage are not a single auditable matrix.
3. **M-03 — native observability and historical comments require review.**
   Error output must remain safe and old commented alternatives should not be
   mistaken for supported paths.

### Low

Legacy benchmark/CLI/mmap code and split one-function translation units increase
archaeology cost. They are not removal candidates without caller/build proof.

## Scheduler, memory, SIMD, ABI, and security conclusions

- **Scheduler:** retain C execution scheduling. Task completion is a linear
  ownership point; client cancellation does not imply native cancellation.
- **Memory:** output release and P7.2.8 state ownership are clear. Arena/slab
  remain justified for native buffers, but need focused stress evidence.
- **SIMD:** CPU detection includes AVX state checks and scalar fallbacks; this
  is not a substitute for operation-specific equivalence evidence.
- **ABI:** ABI v1 is contained, versioned, symbol-checked, and consumer-tested.
  No layout/private observer symbol is installed or exported.
- **Security:** decoder fuzz, malformed-input regressions, real images,
  sanitizer, and sandbox evidence are positive. They do not establish product
  body/pixel/workspace/concurrency limits.

## Verification baseline

The preserved P7.2.8 Ubuntu 24.04 verification passed format-check, normal and
ASAN CTest/regressions, ABI/FFI/supervisor/API tests, real-image verification,
sandbox, and two 5,000-run fuzz targets. This audit adds documentation only;
it does not rerun that gate or convert its bounded fuzzing into a security proof.

## Final classification and next gate

KEEP: ABI v1, C codec path, C scheduler, verified private layout path, build
and verification gate. HARDEN: input ownership boundaries, SIMD-001,
scheduler/allocator stress, observability review. QUARANTINE: mmap decode,
float FIT, partial grid placement. RESEARCH: historical/duplicate source
reachability and any removal decision.

Recommended P7.4 is a bounded hardening phase for H-01/H-02 evidence and the
allocator/scheduler matrices. No public layout ABI, Rust layout FFI, HTTP, or
Nuxt work follows automatically.
