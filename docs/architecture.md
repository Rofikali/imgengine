# IMGENGINE Architecture

**Status:** The Rust/C/Nuxt first-MVP path is implemented and validated in
Ubuntu 24.04 Docker CI. Actual-VPS operation, public release, and retirement of
the retained legacy stack are not yet claimed.

## Implemented first-MVP path

```text
Browser
  -> Nuxt/Vue server proxy
  -> Rust/Axum API
  -> RequestLifecycleService
  -> bounded Rust admission/control
  -> safe Rust FFI
  -> libimgengine C ABI
  -> retained C scheduler
  -> C decode/layout/render/encode engine
  -> completed JPEG response and ephemeral cleanup
```

Rust owns the internet-facing control plane: authentication, multipart and
contract validation, lifecycle state, bounded admission, deadlines, response
abandonment, graceful drain, safe error mapping, and workspace cleanup. C owns
the execution plane: codec integration, layout, rendering, SIMD, encoder
integration, and the retained C scheduler. Nuxt owns presentation and a
same-origin server proxy; browser code never receives the Rust API key or
invokes native code.

The implemented API slices are documented in `api.md`: basic JPEG/PNG render,
FIT-only layout render, and physical-sheet render. Their configured limits are
defensive integration controls, not proven capacity limits.

## Retained legacy path

The repository still contains the historical Nuxt -> FastAPI ->
Celery/Redis/PostgreSQL -> Python worker -> C CLI path. It is retained for
rollback/migration evidence and must not be represented as deleted merely
because the Rust path is verified. It is not the intended first-MVP deployment
topology. Retirement needs route/behavior review, a rollback window, actual
deployment evidence, and an explicit release decision.

## Service boundaries

| Component | Current responsibility | Not its responsibility |
| --- | --- | --- |
| Nuxt/Vue | User experience and server-side same-origin proxy | Native execution or browser-held secrets |
| Rust/Axum | HTTP contract, auth, lifecycle, admission, cleanup | Pixel kernels or scheduler replacement |
| Safe Rust FFI | Narrow ownership-safe native facade | HTTP or product policy |
| C scheduler/engine | Native execution, codecs, layout, render/encode | HTTP, identity, or persistent jobs |
| Caddy profile | TLS/public ingress and reverse proxy | Application authorization |

## Architectural decisions

1. **Synchronous and ephemeral:** Rust routes return a completed JPEG and do
   not create durable jobs, output URLs, or permanent image storage.
2. **Safe FFI is the active native boundary:** ABI v1 and its safe Rust wrapper
   are implemented. The C scheduler remains intentionally retained; Rust
   bounded admission is not a scheduler rewrite.
3. **No premature infrastructure:** the first-MVP path adds no Redis,
   PostgreSQL, Celery, object store, or distributed queue.
4. **No false cancellation claim:** client disconnect, deadline, and drain do
   not cancel an already-running ABI-v1 native operation; they preserve
   lifecycle ownership and cleanup semantics.
5. **Production profile is evidence-incomplete:** the hardened Compose profile
   is implemented, but actual VPS sizing, TLS/DNS/firewall, canary, rollback,
   and operations evidence are still required.

## Non-goals and open release gates

- Rewriting native kernels or the C scheduler in Rust.
- Declaring production readiness from local/Docker CI alone.
- Removing the legacy stack without an approved retirement decision.
- Adding accounts, billing, durable jobs, or persistent storage without a
  separate product and security design.

The canonical release decision record is `MVP_RELEASE_CHECKLIST.md`; the
actual-host evidence procedure is `P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md`.

