# Rust Supervisor Foundation

**Status:** Implemented as a Priority 4 discovery/proof path. It does not
replace the C scheduler, Rust FFI boundary, FastAPI backend, or legacy worker.

## Purpose

`imgengine/rust/imgengine-supervisor` is a synchronous control-plane component
that owns exactly one safe `imgengine::Engine`. It proves the lifecycle rules
needed before scheduler migration or an Axum service is considered:

- one engine and one in-flight operation per process;
- isolated request directories created with Linux `/dev/urandom` names and
  `0700` permissions;
- cleanup after success, native failure, and a post-operation deadline result;
- startup removal of stale `request-*` directories older than five minutes;
- redacted size buckets, result classes, and duration telemetry;
- no image paths, filenames, bytes, engine diagnostics, or native pointers in
  the completion event.

It also provides an in-process bounded admission handle. The handle owns a
fixed-capacity Rust channel to one dedicated worker thread, which creates and
retains the non-`Send` native engine. Full queues reject new owned inputs with
the redacted `overloaded` class instead of accumulating image bytes. The
admission metrics expose accepted, completed, successful, failed, overload,
resource-limit, and deadline counters.

Dropping an accepted request handle does not cancel queued or active work. The
worker completes accepted work during orderly shutdown; this preserves ABI
v1's no-mid-operation-cancellation contract.

The supervisor keeps source bytes and returned JPEG bytes in memory. Its CLI is
a verification consumer only: it writes an explicit caller-owned output path;
the production HTTP path must stream output rather than retain it.

## Deadline Contract

ABI v1 cannot cancel an image operation once it has started. A zero deadline is
rejected before workspace or native work starts. A positive deadline is checked
after the synchronous call; if exceeded, the output is discarded and the
caller receives `deadline_exceeded`. Future process supervision may enforce a
hard wall-clock timeout outside the engine process.

## Non-Goals

This proof intentionally does not add Tokio, Axum, a queue, Redis, PostgreSQL,
or scheduler replacement. The bounded Rust channel is an admission boundary,
not a replacement for the C scheduler. It does not make the opaque engine
`Send` or `Sync`.
The native C scheduler, arena, slab, SIMD, and codec layers remain unchanged.

## Verification

The Ubuntu 24.04 Docker verifier runs the supervisor unit tests and its CLI
against `libimgengine.so`. Tests cover redacted size buckets, stale-workspace
sweeping, deadline admission, malformed input, successful JPEG output,
workspace cleanup, bounded overload rejection, input limits, and cross-thread
admission.
