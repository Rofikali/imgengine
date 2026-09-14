# Rust Request-Lifecycle Boundary

## Status

**Implemented contract and transport-neutral adapter.** This document records the boundary that must be preserved when an Axum adapter is added. It does not introduce an HTTP server or change the native execution plane.

## Architectural Decision

The current production boundary is:

```text
Rust HTTP / application lifecycle
        -> Rust admission / control
        -> C scheduler
        -> C execution engine
```

Rust owns request-local policy and lifecycle behavior. The `AdmissionController` keeps one `Supervisor` and one ABI v1 engine on its worker thread. C continues to own scheduling, decoding, encoding, rendering, SIMD, native memory, and native execution.

This decision preserves the scheduler characterization result in `SCHEDULER_CHARACTERIZATION_REPORT.md`. No request-lifecycle component may create a second engine, call private native APIs, or claim it can interrupt native work.

## Transport-Neutral Contract

`imgengine-supervisor::RequestLifecycleService` is the application boundary. A future HTTP adapter supplies an owned `IncomingRequest` and maps `ApplicationResponse` or `ApplicationFailure` to its transport. The service has no HTTP framework dependency and can be tested without a network.

Accepted content types are `image/jpeg` and `image/png`; parameters are ignored after the media type. The content type is only an early request-boundary check. The C engine remains the authoritative decoder and validates actual content and dimensions.

The request limit applied at this boundary is the stricter of the application policy and the admission-controller limit. This prevents the application layer from retaining an input which the bounded admission layer would reject.

## State Machine

```text
Received
  -> Rejected                 validation, limit, overload, unavailable
  -> Accepted                 body transferred to bounded admission
       -> Completed           Rust-owned JPEG response available
       -> Failed              classified native / lifecycle failure
       -> ResponseAbandoned   client/application stops waiting
```

`Received` and validation are synchronous application transitions. `Accepted` means the body was placed in the bounded Rust admission queue; it does not promise successful native processing. `ResponseAbandoned` applies only to the client response handle: queued or executing work remains owned by the admission worker and completes normally. The worker records completion even if delivery of its response fails.

During orderly shutdown, admission stops new submissions and drains accepted work. A request submitted after shutdown is rejected as `unavailable`. Process termination is outside this graceful guarantee and must be handled by deployment-level shutdown budgets.

## Ownership And Cleanup

| Resource | Owner | Transfer / release rule |
| --- | --- | --- |
| Request ID | lifecycle service | Generated server-side from Linux entropy; safe for correlation, never client supplied. |
| Request body | HTTP adapter, then admission queue | `IncomingRequest` owns bytes; `admit` moves them into the bounded queue; queue release follows processing or rejection. |
| Workspace | `Supervisor` worker | A private, random workspace is created per operation and removed after success or failure. |
| Native input and output | ABI v1 call | C borrows input during the call; the safe Rust `imgengine` wrapper releases native output before returning a Rust `Vec<u8>`. |
| Response body | application/HTTP adapter | `ApplicationResponse` owns the returned JPEG bytes; dropping it frees the bytes. |
| Client disconnect | HTTP adapter | Adapter calls `AdmittedRequest::abandon`, which drops only the response receiver and emits a redacted event. It does not cancel native work. |

Cleanup invariants: user body bytes are not written to logs; native output is always released through the safe wrapper; per-operation workspaces are removed after success, failure, and abandoned responses; shutdown drains accepted work before the worker exits.

## Timeouts And Cancellation

`RequestPolicy` accepts an optional native execution deadline. It is passed unchanged through admission to `Supervisor`. ABI v1 has **no mid-operation cancellation**: the deadline is checked before work begins and after native execution returns. A result that finishes after the deadline is discarded and classified as `deadline_exceeded`; the C operation is never interrupted.

An HTTP adapter may impose a shorter client-response timeout. On expiry or disconnect it abandons the response handle, emits `response_abandoned`, and lets admission/native cleanup finish. It must not represent this as cancellation of the native operation.

## Error Mapping

Only stable classes are exposed outside the application layer. Raw native diagnostics, paths, image bytes, and implementation details are not transport responses or lifecycle events.

| Application error | Result class | HTTP mapping for future adapter |
| --- | --- | --- |
| `invalid_request` | `invalid_argument` | 400 |
| `unsupported_media_type` | `invalid_image` | 415 |
| `payload_too_large`, `resource_limit` | `resource_limit` | 413 |
| `invalid_image` | `invalid_image` | 422 |
| `overloaded` | `overloaded` | 429 |
| `unavailable` | `unavailable` | 503 |
| `deadline_exceeded` | `deadline_exceeded` | 504 |
| `internal` | `internal` | 500 |

The actual HTTP response schema is deliberately deferred to the HTTP-adapter contract. This mapping is an adapter requirement, not an Axum dependency.

## Observability

Every lifecycle event contains only safe fields:

- server-generated `request_id`;
- lifecycle state and stable result class;
- canonical input content type;
- input and output size buckets.

The HTTP adapter must add its trace/correlation fields and record duration, route, client-disconnect count, and response status. Admission metrics remain the source for queue depth, overload, completion, resource-limit, and deadline counters. Never log client filenames, raw headers beyond approved identifiers, filesystem paths, image bytes, secrets, or raw decoder diagnostics.

## Security And Resource Limits

- Require a supported content type before queue admission, but never trust it as image validation.
- Reject an empty body and enforce the strictest configured byte limit before retaining a queue item.
- Preserve native decoder pixel/dimension/resource limits and sandbox behavior.
- Use opaque server-generated request IDs and server-generated workspace names.
- Bound queued bytes through `AdmissionController`; do not add durable storage or a database.
- Map external failures only to stable classes and transport statuses.

## Focused Verification

The lifecycle module tests public Rust application behavior without a network:

- content-type and limit validation;
- application/admission limit composition;
- successful Rust-owned output and workspace cleanup;
- malformed image classification without diagnostic leakage;
- response abandonment without native cancellation;
- orderly shutdown: accepted work drains and new work is rejected.

Linux/Docker validation results are recorded with the implementation change. A future Axum adapter must add transport-level tests for header/body extraction, client disconnect signalling, response timeout, and graceful server shutdown without weakening these invariants.
