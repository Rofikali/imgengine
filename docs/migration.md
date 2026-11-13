# IMGENGINE Rust/C/Nuxt Migration Plan

**Status:** Execution plan. Each phase is reversible until the final retirement phase.

## 1. Current Architecture and Migration Position

The verified first-MVP path is Nuxt -> Rust/Axum ->
`RequestLifecycleService` -> bounded Rust admission -> safe Rust FFI -> C ABI
-> retained C scheduler -> C engine. It provides synchronous, ephemeral
JPEG/PNG-to-JPEG processing and the documented P8/P12 layout slices. It is
validated by the repository's Ubuntu 24.04 Docker CI gates, not by an actual
VPS deployment or public canary.

The historical Nuxt -> FastAPI -> PostgreSQL + Redis/Celery -> Python worker
-> C CLI stack remains present as a legacy/rollback path. It is not the intended
first-MVP deployment topology, but it has not been retired. Do not claim that
the Rust path is the default public service or that the legacy stack was
deleted until the explicit retirement criteria in this document are met.

## 2. Technology Decision

| Technology | Decision | Reason |
| --- | --- | --- |
| C core and CLI | Keep and harden | Proven performance path; preserve SIMD and codec investment. |
| Rust/Axum/Tokio | Implemented for the first-MVP path | Safe control plane, lifecycle, bounded admission, and HTTP transport. |
| Nuxt/Vue/TypeScript | Keep | Existing product UI and server rendering boundary. |
| Python/FastAPI/Celery | Retained legacy path; retire only after approved evidence | Not required for the intended synchronous ephemeral MVP path. |
| Redis/PostgreSQL | Remove from MVP | No durable jobs/accounts/billing in the target MVP. |
| S3/MinIO | Remove from MVP; design an adapter later | No permanent output retention. |
| ELK/Jaeger local stack | Remove from VPS baseline | Cost and operational weight exceed the deployment target. |

## 3. C-to-Rust Boundary

**Keep in C now:** codec integration, decode/render/encode kernels, layout math validated by existing tests, SIMD dispatch, and native benchmarks.

**Implemented in Rust:** HTTP validation, request lifecycle, bounded admission,
filesystem/workspace policy, safe error mapping, and a safe opaque-handle FFI
facade. The current adapter calls the retained C scheduler through that FFI;
it does not execute the C CLI as the active Rust boundary.

**Evaluate later:** scheduler/queues, arena/slab ownership, RPC/VFS, and non-kernel orchestration. Migrate only with explicit ownership semantics and scalar/SIMD equivalence evidence.

## 4. Data Lifecycle

Each request receives a random `0700` directory beneath a configurable private
temporary root. The current P5 path keeps input and returned JPEG bytes in
memory; it does not use client filenames or persist output. The workspace guard
deletes its directory after completion, native failure, deadline, response
abandonment, or shutdown drain, and startup sweeps stale directories older than
five minutes. No database record, object, output URL, image metadata, or
thumbnail survives the request. Metrics and redacted logs may survive under
their separate retention policy. Output-byte and workspace-disk quotas remain
unimplemented, and streaming remains deferred.

## 5. Observability

Emit one JSON record per lifecycle event with `request_id`, `trace_id`, route, result class, duration, byte/pixel buckets, and safe engine outcome. Expose Prometheus metrics privately. Trace propagation is W3C-compatible; exporting is optional and disabled without an endpoint. Alert on readiness failure, sustained errors, overload, timeouts, cleanup failure, high RSS, and native crash rate. Never log filenames, paths, image bytes, keys, sessions, or raw diagnostics without sanitization.

## 6. Staged Delivery

1. **Security and correctness:** fix and regress-test scheduler delivery, real decoded-dimension validation, AVX/XCR0 detection, arena overflow, slab ownership/ASAN behavior, sandbox ordering, and fuzz instrumentation.
2. **Clean C ABI:** make installed headers self-contained; define visibility, symbol/version policy, ownership, error, lifecycle, and compatibility guarantees for `libimgengine`.
3. **Rust FFI:** add a safe Rust wrapper over the stable C ABI with ownership-safe output types and malformed-input integration tests. ABI v1 has no mid-operation cancellation, so request-level deadlines remain outside the wrapper in the Rust lifecycle layer. Do not migrate the scheduler yet.
4. **Move orchestration selectively:** the implemented Rust supervisor,
   bounded admission/control, and transport-neutral request lifecycle own one
   safe engine, ephemeral workspace lifecycle, stale cleanup, request/content
   validation, deadline policy, bounded overload rejection, response
   abandonment, error mapping, and redacted telemetry. Scheduler
   characterization retains the C scheduler; reconsider it only with new
   measured evidence and a separate decision. Keep C scheduling, kernels, and
   memory ownership as the correctness/performance baseline.
5. **Rust SaaS backend:** the narrow Axum/Tokio vertical slice, same-origin
   Nuxt proxies, and P8/P12 additive routes are implemented. Reach release-
   scoped contract review, actual-host evidence, canary, and rollback proof
   before calling it the public default.
6. **Remove obsolete infrastructure:** retire FastAPI, Python workers, Celery, Redis, and job-only PostgreSQL only after the Rust route has passed its rollback window.
7. **Nuxt modernization:** preserve Vue while upgrading the UI to a supported Nuxt 4/5 TypeScript release after backend contract parity.
8. **Benchmark every stage:** measure native and end-to-end latency, throughput, RSS, failure handling, and bounded concurrency to establish the real $5-VPS operating envelope.

## Priority 1 Acceptance Gate

Do not begin ABI stabilization until all of the following pass on Linux with the intended release compiler:

- CTest safety, layout, SIMD-equivalence, and generated-pipeline tests.
- ASAN/UBSAN build and test run with leak detection enabled.
- Bounded decoder fuzzing with the core library instrumented, not only the fuzz harness.
- Scheduler overflow regression proving a task reaches a worker from the fallback MPMC queue.
- Security regressions covering malformed inputs and actual decoded dimensions.
- Sandbox-enabled smoke test proving initialization, render, shutdown, and denied network/process syscalls.

The Windows/MSYS build is a portability signal, but the current native implementation includes POSIX `mmap` sources and is not the release verification environment. Fixing that platform boundary is separate work; it must not be hidden by a passing syntax-only check.

## Exit Criteria

The Rust path becomes default only when contract tests pass, native release gates pass, security boundary tests pass, cleanup is proven for success/failure/cancellation, production canary meets latency/error/memory targets, rollback is rehearsed, and no persistent image artifact is created by the target path.
