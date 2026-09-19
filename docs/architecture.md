# IMGENGINE Target Architecture

**Status:** Planned migration target. It does not describe the currently deployed Python/Celery stack.

## Current Architecture

The current product is Nuxt 3, FastAPI, Celery/Redis, PostgreSQL, a Python worker, local-or-S3 storage, and the C `imgengine_cli`. A submitted upload is persisted, queued, rendered by the CLI, and retained for 24 hours. This provides useful async behavior, but it is too many services and too much persistent data for the intended low-cost, ephemeral product.

The native engine is a C11 shared library and CLI. Its decode, layout, JPEG/PDF encode, SIMD dispatch, fuzzing, sanitizer, regression, ABI, and benchmark work is the performance foundation to retain.

## Target System

```text
Browser
  -> Nuxt/Vue TypeScript UI
  -> Rust Axum API (same-origin API or server proxy)
  -> bounded Tokio execution supervisor
  -> non-root C engine process (initial production boundary)
  -> response download stream
```

Rust owns the control plane: HTTP, authentication, request limits, upload streaming, temporary-file lifecycle, scheduling, audit-safe logs, metrics, and engine supervision. C owns pixel-heavy decode/render/encode kernels. Nuxt owns the user experience only; it never exposes a privileged service key or invokes native code.

The first Rust production path executes the existing C CLI as a non-root child process. Process isolation is intentional: untrusted image parsing remains outside the HTTP server. A Rust FFI adapter is a later optimization after ABI hardening and parity evidence; it is not a migration prerequisite.

## Service Shape

| Component | Responsibility | Excluded from the MVP |
| --- | --- | --- |
| Nuxt | Upload, layout controls, progress, download UX | Secrets, native execution, durable job ownership |
| Rust/Axum | API versioning, auth, validation, limits, lifecycle, telemetry | Pixel manipulation and long-lived artifact storage |
| Tokio supervisor | Bounded concurrency, timeouts, process exit handling, cleanup | Durable queue semantics |
| C engine | Decode, layout, resize, SIMD, encode | HTTP, user identity, filesystem policy |
| Caddy/reverse proxy | TLS, HTTP security headers, public ingress | Application authorization |

## Architecture Decisions

1. **Synchronous first:** `POST /api/v1/render` holds the request open while one bounded render completes, then streams the result. This removes the need for Redis, Celery, PostgreSQL, and durable job artifacts on a small VPS.
2. **Ephemeral by default:** upload, intermediate, output, and engine diagnostics live only in a per-request directory for at most five minutes. No output URL is durable.
3. **No direct FFI initially:** keep the CLI isolation boundary while public C headers, exported symbols, allocation ownership, and failure behavior become ABI-safe.
4. **No dual writers:** the legacy Python service and Rust service must not both mutate a shared job-state store. Compatibility is provided at the HTTP edge, not by competing workers.
5. **Optional persistence later:** accounts, billing, usage reporting, or asynchronous jobs may justify PostgreSQL. Object storage may be reintroduced behind a storage trait only when a product requirement needs it.

## Native Boundary Contract

The C execution boundary accepts only a generated request directory and validated layout arguments, writes only a generated output path inside that directory, and returns a bounded diagnostic summary. Rust supplies no user filename or raw path to the CLI. The supervisor enforces wall-clock, CPU, memory, file-size, process-count, and output-size limits, then removes the directory on every exit path.

Before FFI, the C library must have installed self-contained public headers, hidden-by-default symbols, an explicit export list/version policy, stable ownership APIs, and Rust integration tests covering malformed encoded input, cancellation, repeated initialization, and scalar/SIMD output equivalence.

## Non-Goals

- Rewriting SIMD kernels in Rust.
- Promising accounts, billing, S3, async queues, or multi-node scheduling before the product needs them.
- Treating a Docker Compose development stack as a production deployment design.

