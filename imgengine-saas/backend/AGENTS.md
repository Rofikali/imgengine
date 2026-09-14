# AGENTS.md — IMGENGINE SaaS Backend

## 1. Scope

These instructions apply to:

```
/imgengine-saas/backend
```

They supplement the root repository `AGENTS.md`.

The target backend architecture is Rust/Axum/Tokio.

The backend must remain a control-plane/orchestration layer.

It must not duplicate the native image-processing engine.

---

# 2. Target Architecture

The target request path is:

```
Client
  |
  v
Edge / Cloudflare where appropriate
  |
  v
Rust Axum API
  |
  v
Rust application/request lifecycle
  |
  v
Rust bounded admission/control
  |
  v
Safe Rust imgengine wrapper
  |
  v
libimgengine ABI v1
  |
  v
C scheduler
  |
  v
C native engine
  |
  v
JPEG / PDF result
  |
  v
Stream result
  |
  v
Cleanup
```

The backend should not become a second image-processing engine.

---

# 3. Current Migration Position

The repository is migrating from:

```
Nuxt
  ->
FastAPI
  ->
PostgreSQL / Redis / Celery
  ->
Python worker
  ->
C engine
```

toward:

```
Nuxt
  ->
Rust/Axum
  ->
Rust request lifecycle / bounded admission
  ->
safe FFI / libimgengine ABI v1
  ->
C scheduler
  ->
C native engine
```

Completed boundaries are: stable ABI v1, safe Rust FFI, Rust supervisor,
bounded Rust admission/control, scheduler characterization, and the
transport-neutral Rust request lifecycle. Production Axum, legacy retirement,
and distributed scaling remain planned.

---

# 4. Migration Order

Backend implementation should follow:

```
Implemented ABI/FFI/lifecycle/admission boundaries
    ->
Small HTTP adapter around the request lifecycle contract
    ->
API contract parity and integration tests
    ->
Canary
    ->
Legacy retirement
```

Do not start a full production Axum implementation as part of lower-boundary
work. HTTP is an adapter around the application contract, not the location of
lifecycle/business logic.

---

# 5. Rust Responsibilities

Rust should own:

* HTTP handling,
* request validation,
* authentication/authorization when introduced,
* request lifecycle,
* application-level request/disconnect handling,
* concurrency limits,
* rate limiting,
* filesystem policy,
* temporary workspace lifecycle,
* process supervision where required,
* structured logging,
* metrics,
* tracing,
* safe C FFI wrapper,
* API error mapping.

The Rust supervisor/admission layer is NOT a replacement for the C scheduler.
Rust owns bounded admission and overload behavior around native execution; C
owns native scheduling and worker execution.

C continues to own native scheduling, worker execution, image decoding and
encoding, rendering/layout, SIMD, and native memory/execution internals. Do
not duplicate these responsibilities in Rust application or HTTP code.

Rust should not own native image kernels simply for architectural purity.

---

# 6. FFI Boundary

Treat FFI as a security and correctness boundary.

The Rust wrapper should:

* validate arguments,
* enforce ownership,
* enforce lifetimes,
* translate C errors,
* prevent invalid handles,
* release resources reliably,
* isolate `unsafe`,
* document safety assumptions.

Avoid spreading raw C pointers throughout application code.

Prefer:

```
unsafe FFI module
      |
      v
safe domain wrapper
      |
      v
application logic
```

The rest of the Rust application should ideally not need to understand raw C pointers.

---

# 7. Unsafe Code Policy

Every `unsafe` block should have a clear reason.

Before using `unsafe`, ask:

1. Why is unsafe required?
2. What invariant makes it safe?
3. Who guarantees that invariant?
4. What happens if the invariant is violated?
5. Can the same result be achieved safely?

Keep unsafe code small.

Do not use unsafe merely to avoid designing ownership properly.

---

# 8. Thread Safety

Do not automatically assume a C engine handle is:

```
Send
Sync
```

Rust must inherit the real thread-safety properties of the C implementation.

If a native object is not proven thread-safe:

* keep it on the appropriate thread,
* serialize access,
* create separate instances,
* or redesign the lifecycle.

Never add `unsafe impl Send` or `unsafe impl Sync` simply to satisfy the compiler.

---

# 9. Request Validation

Treat all HTTP input as untrusted.

Validate:

* request size,
* image size,
* image dimensions,
* output limits,
* layout parameters,
* MIME/content type,
* timeout,
* concurrency limits,
* supported formats,
* resource consumption.

Do not trust:

* filenames,
* extensions,
* client-provided dimensions,
* client-generated paths,
* arbitrary headers.

Actual decoded dimensions must ultimately be established by the appropriate native validation boundary.

---

# 9.1 Request Lifecycle Contract

The transport-neutral request lifecycle contract is implemented in the Rust
supervisor layer and documented in `docs/RUST_REQUEST_LIFECYCLE.md`. It owns
request IDs/correlation, content-type and byte validation, lifecycle state,
admission decision, deadline policy, ephemeral workspace/output lifecycle,
safe error mapping, and redacted events.

An HTTP server must adapt to this contract rather than reimplement it. The
application layer must remain testable without a network server.

---

# 10. Ephemeral Workspace

Each request should receive an isolated temporary workspace.

Desired properties:

* random directory name,
* restrictive permissions,
* no user-controlled filesystem paths,
* server-generated filenames,
* bounded disk usage,
* cleanup guard,
* cleanup on normal completion,
* cleanup on failure,
* cleanup on timeout,
* cleanup on cancellation,
* cleanup after client disconnect,
* startup stale cleanup.

Target retention:

```
<= 5 minutes
```

Images and generated outputs must not become permanent application state.

---

# 11. No Database for Ephemeral Jobs

The target MVP does not require PostgreSQL for temporary image-processing jobs.

Do not add durable database state merely to represent:

* uploaded image,
* processing job,
* temporary output,
* thumbnail,
* processing queue.

If future product features require accounts, billing, history, quotas, or durable metadata, design that separately.

Do not accidentally turn ephemeral rendering into a durable job platform.

---

# 12. No Redis Initially

Do not introduce Redis simply because:

* queues are common,
* rate limiting is common,
* caching is common,
* tutorials use Redis.

The initial architecture should use in-process bounded concurrency and appropriate OS/runtime primitives where sufficient.

Queue saturation must produce explicit overload behavior; it must not retain
unbounded request bodies or silently block callers.

Redis may be reconsidered later when measured requirements justify it.

---

# 13. API Contract

API behavior must be defined before implementation.

Document:

* endpoint,
* method,
* request,
* response,
* content types,
* size limits,
* errors,
* status codes,
* timeout behavior,
* cancellation behavior,
* output semantics.

Prefer versioned contracts.

Do not allow frontend behavior to become an undocumented backend contract.

---

# 14. Error Handling

External clients should receive safe, stable errors.

Do not expose:

* Rust panic messages,
* C stack traces,
* filesystem paths,
* compiler diagnostics,
* allocator details,
* internal symbols,
* secrets.

Map internal errors into stable application-level error classes.

For example:

```
invalid_request
unsupported_format
resource_limit
timeout
cancelled
engine_error
unavailable
```

Exact names should follow the canonical API contract.

---

# 15. Cancellation

Cancellation must be a first-class design concern.

A client disconnect should not automatically leave:

* native work,
* temporary files,
* memory,
* worker tasks

running indefinitely.

Where native cancellation is unavailable, the supervisor must define bounded behavior.

Every long-running operation needs an upper bound.

ABI v1 does not support mid-operation native cancellation. Model client
cancellation as Rust/application response abandonment, preserve cleanup, and
never claim that an in-flight C operation was cancelled. Orderly shutdown must
stop new submissions and drain accepted work within its documented budget.

---

# 16. Concurrency

Do not allow unlimited native work.

Use explicit limits for:

* request concurrency,
* native execution concurrency,
* memory-heavy operations,
* temporary disk consumption.

The purpose is to protect:

* latency,
* memory,
* CPU,
* server availability.

A server that accepts unlimited requests is not necessarily scalable.

---

# 17. Resource Limits

Eventually measure and enforce limits for:

* upload bytes,
* decoded pixels,
* output dimensions,
* CPU time,
* wall-clock time,
* temporary storage,
* concurrent jobs,
* request rate.

Limits must be based on evidence where possible.

The current boundary already composes application and admission input-byte
limits using the stricter limit. Continue to bound temporary storage and
concurrency; do not add permanent image storage for ephemeral jobs.

Do not select arbitrary "million-user" limits without capacity measurements.

---

# 18. Observability

Use structured logging.

A request lifecycle should be traceable using safe identifiers such as:

* request ID,
* trace ID,
* route,
* operation,
* result class,
* duration.

Metrics should eventually include:

* requests,
* successes,
* failures,
* timeouts,
* cancellations,
* overload,
* native errors,
* processing latency,
* active requests,
* cleanup failures,
* memory/resource pressure.

Never log:

* uploaded image bytes,
* secrets,
* API keys,
* cookies,
* authentication tokens,
* private filesystem paths,
* raw user filenames unless explicitly safe.

---

# 19. Tracing

Use W3C-compatible trace propagation where practical.

External tracing exporters are optional.

Do not make the MVP dependent on a large tracing infrastructure stack.

The backend should remain functional when no tracing collector is configured.

---

# 20. Performance

Measure before optimizing.

Important metrics:

* request latency,
* native processing latency,
* P50,
* P95,
* P99,
* throughput,
* CPU usage,
* memory/RSS,
* temporary disk usage,
* concurrent capacity,
* error rate.

Separate:

```
HTTP overhead
```

from:

```
native processing cost
```

and:

```
total request cost.
```

This allows the team to identify the actual bottleneck.

---

# 21. Economic Engineering

Backend design must consider infrastructure economics.

Eventually calculate:

```
server_cost / processed_images
```

and:

```
processing_cost / customer
```

Then compare with:

```
revenue / customer
```

and:

```
gross_margin
```

Do not build an architecture that requires expensive distributed infrastructure before the product has revenue to justify it.

The initial target is a low-cost VPS deployment.

---

# 22. Security

The backend is an internet-facing attack surface.

Protect against:

* oversized requests,
* malformed image uploads,
* path traversal,
* filesystem abuse,
* resource exhaustion,
* request floods,
* timeout attacks,
* malformed parameters,
* unsafe error disclosure,
* SSRF where relevant,
* arbitrary process execution,
* privilege escalation.

The backend should run with least privilege.

Do not run the application as root unless there is a documented and unavoidable requirement.

---

# 23. Cloudflare / Edge

Cloudflare or another edge layer may provide:

* TLS termination,
* WAF,
* rate limiting,
* edge filtering,
* request protection.

Do not assume the edge layer is the only security boundary.

The backend must remain secure if a request reaches it directly.

---

# 24. Docker

Docker images should be:

* minimal,
* reproducible,
* non-root where practical,
* explicit about runtime dependencies,
* free of development secrets.

Do not place:

* API keys,
* passwords,
* production credentials

inside images.

Use environment/runtime secret management.

---

# 25. Testing Strategy

Rust backend tests should eventually cover:

### Unit

* validation,
* error mapping,
* configuration,
* lifecycle helpers.

### Integration

* HTTP contract,
* FFI wrapper,
* temporary workspace,
* cleanup,
* cancellation.

### Security

* malformed inputs,
* oversized input,
* path traversal attempts,
* resource limits,
* error disclosure.

### Failure

* engine failure,
* timeout,
* client disconnect,
* cancellation,
* allocation/resource failure.

### End-to-end

```
HTTP
  ->
Rust
  ->
FFI
  ->
C engine
  ->
output
  ->
cleanup
```

---

# 26. Contract Parity

Before replacing the legacy FastAPI path, establish parity for relevant behavior.

Compare:

* request validation,
* supported formats,
* dimensions,
* layout behavior,
* errors,
* status codes,
* output format,
* cleanup,
* timeout behavior.

Do not assume two implementations are equivalent because they produce an HTTP 200.

---

# 27. Legacy Python/FastAPI Retirement

Do not delete the old stack at the beginning of Rust migration.

Retirement requires:

1. Rust implementation,
2. contract parity,
3. integration tests,
4. security verification,
5. performance verification,
6. canary,
7. rollback window,
8. production evidence,
9. explicit retirement.

Only then remove:

* FastAPI,
* Python workers,
* Celery,
* Redis,
* job-only PostgreSQL dependencies.

---

# 28. Backend Code Organization

Prefer clear separation between:

```
HTTP layer
    |
    v
Application layer
    |
    v
Domain / request model
    |
    v
Native engine facade
    |
    v
FFI
    |
    v
C ABI
```

Do not put native pointer manipulation directly inside HTTP handlers.

Do not put business logic inside route definitions.

Do not make the FFI wrapper responsible for HTTP concerns.

---

# 29. No Premature Distributed Architecture

Do not add:

* Kafka,
* Kubernetes,
* distributed job queues,
* service mesh,
* microservices,
* Redis clusters,
* PostgreSQL clusters

unless measured requirements justify them.

The first goal is a reliable single-service architecture that can later scale horizontally.

Horizontal scalability should come from:

```
stateless application
   +
bounded ephemeral processing
   +
no permanent image state
```

where appropriate.

---

# 30. Definition of Done for Rust Backend

A backend task is complete only when applicable:

* API contract is defined,
* implementation is complete,
* validation is tested,
* FFI safety is reviewed,
* ownership is clear,
* cancellation is handled,
* cleanup is tested,
* resource limits are tested,
* security tests pass,
* integration tests pass,
* relevant performance measurements exist,
* documentation is updated,
* no unrelated infrastructure is introduced,
* git diff is reviewed.

---

# 31. What NOT To Do

Do not:

* start a full rewrite without a migration plan,
* recreate FastAPI architecture in Rust line-for-line,
* recreate Celery unnecessarily,
* add Redis because a queue "might be needed",
* add PostgreSQL because every SaaS "should have a database",
* permanently store images,
* put unsafe C operations throughout the application,
* claim Rust is faster without benchmarks,
* claim millions of users without capacity testing.

---

# 32. Backend Engineering Principle

The Rust backend should be:

> Small, stateless, safe, observable, cancellable, resource-bounded, and economically efficient.

Its job is to safely move requests to the native engine and return results.

It should not become a second monolith containing every possible technology.
