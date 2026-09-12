# IMGENGINE Rust/C/Nuxt Migration Plan

**Status:** Execution plan. Each phase is reversible until the final retirement phase.

## 1. Current Architecture and Problems

Current production behavior is Nuxt -> FastAPI -> PostgreSQL + Redis/Celery -> Python worker -> C CLI, with local/S3 artifacts and 24-hour retention. It provides an asynchronous job API, but introduces durable state, shared browser service identity, multiple root-running services, a heavy optional observability stack, and cleanup gaps. The C core has strong test investment, but its installed ABI/FFI boundary and several concurrency/memory/architecture issues are not ready to be treated as a stable Rust library boundary.

## 2. Technology Decision

| Technology | Decision | Reason |
| --- | --- | --- |
| C core and CLI | Keep and harden | Proven performance path; preserve SIMD and codec investment. |
| Rust/Axum/Tokio | Introduce | Safe control plane, ownership, cancellation, bounded process supervision. |
| Nuxt/Vue/TypeScript | Keep | Existing product UI and server rendering boundary. |
| Python/FastAPI/Celery | Retire after parity | Not required for synchronous ephemeral rendering. |
| Redis/PostgreSQL | Remove from MVP | No durable jobs/accounts/billing in the target MVP. |
| S3/MinIO | Remove from MVP; design an adapter later | No permanent output retention. |
| ELK/Jaeger local stack | Remove from VPS baseline | Cost and operational weight exceed the deployment target. |

## 3. C-to-Rust Boundary

**Keep in C now:** codec integration, decode/render/encode kernels, layout math validated by existing tests, SIMD dispatch, and native benchmarks.

**Move to Rust first:** CLI option parsing, HTTP validation, request lifecycle, filesystem policy, process supervision, rate/concurrency controls, logging/metrics, and a future safe opaque-handle FFI facade.

**Evaluate later:** scheduler/queues, arena/slab ownership, RPC/VFS, and non-kernel orchestration. Migrate only with explicit ownership semantics and scalar/SIMD equivalence evidence.

## 4. Data Lifecycle

Each request receives a random `0700` directory beneath a configurable private temporary root. Upload, normalized input, output, and bounded diagnostics are created there with server-generated names. A guard deletes the directory after streaming, client disconnect, timeout, engine error, or panic. A startup sweeper deletes stale directories older than five minutes. No database record, object, output URL, image metadata, or thumbnail survives the request. Metrics and redacted logs may survive under their separate retention policy.

## 5. Observability

Emit one JSON record per lifecycle event with `request_id`, `trace_id`, route, result class, duration, byte/pixel buckets, and safe engine outcome. Expose Prometheus metrics privately. Trace propagation is W3C-compatible; exporting is optional and disabled without an endpoint. Alert on readiness failure, sustained errors, overload, timeouts, cleanup failure, high RSS, and native crash rate. Never log filenames, paths, image bytes, keys, sessions, or raw diagnostics without sanitization.

## 6. Staged Delivery

1. **Security and correctness:** fix and regress-test scheduler delivery, real decoded-dimension validation, AVX/XCR0 detection, arena overflow, slab ownership/ASAN behavior, sandbox ordering, and fuzz instrumentation.
2. **Clean C ABI:** make installed headers self-contained; define visibility, symbol/version policy, ownership, error, lifecycle, and compatibility guarantees for `libimgengine`.
3. **Rust FFI:** add a safe Rust wrapper over the stable C ABI with ownership-safe output types, cancellation and malformed-input integration tests, and no scheduler migration yet.
4. **Move orchestration selectively:** benchmark and prove safety before moving scheduler and then memory ownership from C to Rust. Keep C kernels as the correctness/performance baseline.
5. **Rust SaaS backend:** introduce Axum/Tokio only after the native boundary is stable; reach contract parity before Nuxt cutover.
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
