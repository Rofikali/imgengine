# Rust Request-Lifecycle Boundary

## Status

**Implemented contract and transport-neutral adapter.** The narrow P5 Axum
adapter consumes this boundary; this document records the part that remains
HTTP-framework-independent. It does not change the native execution plane.

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

`imgengine-supervisor::RequestLifecycleService` is the application boundary.
The P5 Axum adapter supplies an owned `IncomingRequest` and maps
`ApplicationResponse` or `ApplicationFailure` to HTTP. The service itself has
no HTTP framework dependency and can be tested without a network.

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

P6.3 gives the Axum runtime one graceful-shutdown owner. It atomically moves
the runtime from `Running` to `Draining`, so the existing lifecycle admission
linearization rejects later work as `unavailable`; it then stops listener
acceptance, drains accepted work through the existing worker, joins that worker,
and marks the runtime `Stopped`. A request already in a live handler but not
yet admitted receives `503 unavailable`. A connection after listener closure
may fail at the transport layer rather than receive HTTP. Process termination is
outside this graceful guarantee and must be handled by deployment-level shutdown
budgets.

## Ownership And Cleanup

| Resource | Owner | Transfer / release rule |
| --- | --- | --- |
| Request ID | lifecycle service | Generated server-side from Linux entropy; safe for correlation, never client supplied. |
| Request body | HTTP adapter, then admission queue | `IncomingRequest` owns bytes; `admit` moves them into the bounded queue; queue release follows processing or rejection. |
| Workspace | `Supervisor` worker | A private, random workspace is created per operation and removed after success or failure. |
| Native input and output | ABI v1 call | C borrows input during the call; the safe Rust `imgengine` wrapper releases native output before returning a Rust `Vec<u8>`. |
| Response body | application/HTTP adapter | `ApplicationResponse` owns the returned JPEG bytes; dropping it frees the bytes. |
| Client disconnect after admission | HTTP adapter / lifecycle waiter | Axum handler cancellation signals `AdmittedRequest::wait_or_abandon`, which drops only the response receiver and emits one redacted event. It does not cancel native work. A disconnect before admission creates no lifecycle request. |

Cleanup invariants: user body bytes are not written to logs; native output is always released through the safe wrapper; per-operation workspaces are removed after success, failure, and abandoned responses; shutdown drains accepted work before the worker exits.

## Timeouts And Cancellation

`RequestPolicy` accepts an optional native execution deadline. It is passed unchanged through admission to `Supervisor`. ABI v1 has **no mid-operation cancellation**: the deadline is checked before work begins and after native execution returns. A result that finishes after the deadline is discarded and classified as `deadline_exceeded`; the C operation is never interrupted.

P6.4 makes the timing boundary explicit: queue wait, multipart parsing, and
HTTP response delivery are outside this deadline. It begins after bounded
admission dequeues the owned input, immediately before `Supervisor::process`.
The measured duration includes native execution and workspace cleanup. A
deadline therefore cannot interrupt C work, cannot shorten graceful drain, and
does not establish an HTTP end-to-end timeout. The current 10-second API value
is provisional.

The P6.2 adapter observes cancellation of an in-flight Axum handler after
admission, signals response abandonment, and lets admission/native cleanup
finish. This loopback TCP evidence must not be represented as native
cancellation or as confirmation that response bytes were delivered. A future
client-response timeout policy remains separate work.

## Error Mapping

Only stable classes are exposed outside the application layer. Raw native diagnostics, paths, image bytes, and implementation details are not transport responses or lifecycle events.

| Application error | Result class | P5 HTTP mapping |
| --- | --- | --- |
| `invalid_request` | `invalid_argument` | 400 |
| `unsupported_media_type` | `invalid_image` | 415 |
| `payload_too_large`, `resource_limit` | `resource_limit` | 413 |
| `invalid_image` | `invalid_image` | 422 |
| `overloaded` | `overloaded` | 429 |
| `unavailable` | `unavailable` | 503 |
| `deadline_exceeded` | `deadline_exceeded` | 504 |
| `internal` | `internal` | 500 |

The P5 adapter returns the documented stable problem response; broader response
schema and streaming work remain deferred. This mapping is an adapter concern,
not a dependency of the lifecycle crate.

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

## P6.4 Limit Inventory

The current HTTP adapter has a tested, provisional 1 MiB body/lifecycle/
admission limit and a tested, provisional queue capacity of one. Native
validation independently rejects decoded dimensions above 16,384 in either
axis, more than 268,435,456 pixels, unsafe compression ratios, and RGBA memory
estimates above 4 GiB. This is a native security guard, not a selected product
pixel limit.

Returned JPEGs remain fully in memory with no output-byte ceiling. Workspaces
are random private directories with cleanup and a five-minute startup stale
sweep, but have no byte accounting, aggregate disk quota, or periodic sweeper.
Response/write budgets and graceful-drain duration budgets are also deferred.
None of these absences may be represented as an enforced production limit.

## Focused Verification

The lifecycle module tests public Rust application behavior without a network:

- content-type and limit validation;
- application/admission limit composition;
- successful Rust-owned output and workspace cleanup;
- malformed image classification without diagnostic leakage;
- response abandonment without native cancellation;
- orderly shutdown: accepted work drains and new work is rejected;
- an accepted deadline result drains, cleans up, and returns `504` without
  cancelling native work.

Linux/Docker validation results are recorded with the implementation change.
P6.2/P6.3 add real loopback TCP evidence for client-disconnect signalling and
graceful server shutdown. Response-timeout policy, response-write delivery,
deployment termination budgets, and production capacity remain separate work.
