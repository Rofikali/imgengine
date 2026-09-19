# API Contract and Parity Design Gate

**Status:** P5 first-slice contract. The authenticated Axum adapter described
below is implemented and verified in the Ubuntu 24.04 Docker gate through its
in-process Axum-to-native integration suite. It is neither production-ready
nor a legacy API replacement.

## Decision

The legacy API and Rust API are different product generations. The legacy API
is asynchronous and durable; the proposed Rust API is synchronous and
ephemeral. They are not wire-compatible and must coexist until an explicit
migration, canary, rollback window, and retirement decision.

The current Rust capability is only `image.jpeg_encode`: encoded JPEG or PNG
input produces Rust-owned JPEG output. ABI v1 and `RequestLifecycleService` do
not model layout parameters, presets, PDF output, durable jobs, or incremental
producer-to-client streaming. The first Axum slice must not claim them.

```text
HTTP adapter -> RequestLifecycleService -> bounded Rust admission
             -> C scheduler -> libimgengine ABI v1 -> C engine
```

## Legacy contract: IMPLEMENTED

| Concern | Behavior |
| --- | --- |
| Submit | `POST /api/generate`, `multipart/form-data` |
| Authentication | Required `X-API-Key`; configured-key match is fingerprinted for job ownership |
| File | Required `file`, declared `image/jpeg` or `image/png`; first chunk must have matching signature |
| Fields | Optional `preset`; `cols`, `rows`, `gap`, `padding`, `width`, `height`, `dpi`, `border`, `bleed`, `crop_mark`, `crop_thickness`, `crop_offset` are bounded by `GenerateJob` |
| Limits | `MAX_UPLOAD_BYTES`, default 20 MiB, during read; default rate is `30/minute` by API-key fingerprint or source address |
| Idempotency | Optional `Idempotency-Key`, maximum 128 characters; same owner/key replays the durable job; DB uniqueness resolves races |
| Success | Implicit `200` JSON: `job_id`, `trace_id`, `status`, `output_url`, `expires_at`, `error`; initial state is `queued` |
| Poll/output | `GET /api/status/{job_id}` and `GET /api/output/{job_id}` require owner key; output is `409` before completion and `410` after expiry |
| Lifecycle | Durable DB/Celery: queued, processing, retrying, completed, failed, expired |
| Failure | Auth `401`, type/signature `415`, byte limit `413`, layout validation `422`, broker/storage `503`; other exact framework error bodies are UNKNOWN |
| Retention | Durable artifacts/job metadata until expiry, default about 24 hours |

The Nuxt frontend proxies, polls, and downloads this legacy job API. It is not
already a client of the Rust endpoint.

## Rust API contract: P5 IMPLEMENTED / broader target PLANNED

### Request

`POST /api/v1/render` is a new endpoint. The first vertical slice accepts one
`multipart/form-data` file part named `file`. It accepts `image/jpeg` and
`image/png`; content-type parameters are ignored. Missing `file`, an empty
body, unsupported media type, and an input larger than the effective limit are
rejected before admission.

**Implemented multipart policy:** exactly one `file` field is accepted.
Missing `file`, duplicate `file`, and every other multipart field are rejected
as `400 invalid_request`. Client filenames are ignored and never become paths.

Authentication is required before body admission. The approved first-slice
contract uses `X-API-Key` and process-configured server-side credentials; its
configuration and failure behavior are defined in the P5 readiness decision
below. Sophisticated client-frequency rate limiting is deferred. The adapter
enforces its HTTP body limit while reading, then supplies only owned bytes and
canonical content type to `IncomingRequest`.

The effective limit is the minimum of HTTP body, `RequestPolicy`, and
`AdmissionController` limits. The latter two already compose in the lifecycle.
Production byte, decoded-pixel, output, disk, and deadline limits are PLANNED
capacity policy. Layout/preset fields, client filenames, PDF, and arbitrary
output formats are out of scope until backed by a versioned request model and
C ABI capability.

### Success

Return `200 OK`, `Content-Type: image/jpeg`, complete JPEG bytes, and
`X-Request-Id` from the server-generated lifecycle ID. `X-Trace-Id` and a safe
download filename are PLANNED. `ApplicationResponse` currently holds a full
`Vec<u8>`: the adapter may send that completed buffer with known
`Content-Length`, but this is not producer-to-client streaming.

### Errors

Errors use the implemented `application/problem+json` shape:

```json
{"type":"https://imgengine.example/problems/invalid-image","title":"Invalid image","status":422,"code":"invalid_image","request_id":"server-generated-id"}
```

`imgengine.example` is a deliberately non-production problem-type namespace;
clients must classify errors by the stable `code` and HTTP status rather than
by treating that URI as a live public service. Responses never contain native
diagnostics, paths, filenames, image bytes, secrets, or internal identifiers.

| Application error | HTTP | Code |
| --- | ---: | --- |
| InvalidRequest | 400 | `invalid_request` |
| UnsupportedMediaType | 415 | `unsupported_media_type` |
| PayloadTooLarge, ResourceLimit | 413 | `resource_limit` |
| InvalidImage | 422 | `invalid_image` |
| Overloaded | 429 | `overloaded` |
| Unavailable | 503 | `unavailable` |
| DeadlineExceeded | 504 | `deadline_exceeded` |
| Internal | 500 | `internal` |

Missing or invalid credentials receive the same generic
`401 authentication_failed` problem. A future rate-limit rejection is distinct
from bounded-admission `429 overloaded`; no rate limiter is in the first slice.

## Cancellation, data lifecycle, and shutdown

ABI v1 cannot cancel native work mid-operation. P6.2 verifies real loopback
TCP client disconnect after lifecycle admission: Axum handler cancellation
signals response abandonment, the lifecycle records one redacted
`ResponseAbandoned` event, and queued or active native work continues through
normal cleanup. This is response-delivery abandonment only, never C
cancellation. A disconnect while upload/multipart parsing is still
pre-admission transport behavior: it creates no `AdmittedRequest`, native
operation, workspace, or abandonment event.

The supervisor creates random private `0700` workspaces, uses server-generated
names, removes them after success, failure, deadline, or abandonment, and
sweeps stale directories at startup. Target stale retention is at most five
minutes. The new route creates no durable job, image, output, or image metadata.

P6.3 verifies the real loopback Axum graceful-shutdown sequence: runtime state
becomes `Draining`, new lifecycle admission closes at that state transition,
the listener stops accepting, and previously accepted active and queued work
drains in the existing worker order before its sole join owner marks the runtime
`Stopped`. A request already executing in a live handler when drain begins is
rejected as `503 unavailable` before admission. A connection attempted after
listener closure is a connection-level failure, not an asserted HTTP `503`.
Executing native work is not interrupted. Deployment/process termination and
response-socket budgets remain outside this graceful-drain evidence.

## Parity matrix

| Concern | Legacy | Rust target | Compatible? | Migration decision |
| --- | --- | --- | --- | --- |
| Endpoint | `/api/generate` | `/api/v1/render` | No | Versioned coexistence |
| Auth | Implemented API key | Required `X-API-Key`; process-configured contract defined | Partial | Implement and test the defined contract |
| Input | File, layout, preset | File only, JPEG/PNG encode | No | Deliberately narrower |
| Validation | MIME signature, layout, limits | Type/body/admission/native decode | Partial | Capability-backed fields only |
| Job model | Durable async job | None, synchronous | No | Frontend migration |
| Idempotency | Durable replay | No durable replay specified | No | Decide or omit |
| Output | Poll/download or redirect | Completed JPEG `200` | No | New client contract |
| Errors | FastAPI details partly unfrozen | Stable problem document | No | Add HTTP cases |
| Cancellation | Worker timeout/retry | Response abandonment, no native cancel | No | Intentional distinction |
| Persistence/retention | DB/storage, about 24 hours | None; stale cleanup <= 5 min | No | Intentional MVP change |
| Rate/overload | Rate limit, broker failure | Deferred rate policy, bounded `429` overload | No | Implement admission; defer rate limiting |
| Shutdown | Durable queue/retry | Reject new, drain accepted | No | Set drain budget |

## Executable HTTP acceptance cases

1. Valid JPEG returns `200`, valid JPEG bytes, `Content-Length`, and server ID.
2. Valid PNG returns JPEG and leaves no workspace after completion.
3. Malformed claimed PNG returns `422 invalid_image` without diagnostics.
4. Unsupported type returns `415` before admission.
5. Missing `file` and empty file return `400`.
6. Layout/preset fields are rejected until capability-backed; legacy parity is not implied.
7. Oversized multipart input returns `413` before queue retention.
8. Full admission queue returns `429 overloaded` without retaining another body.
9. Safe-wrapper/native failure returns its mapped problem without disclosure.
10. Post-operation deadline returns `504`, discards output, and claims no interruption.
11. **P6.2 verified:** real loopback TCP disconnect after admission records
    exactly one response-abandonment event while native work completes and
    cleans up; disconnect during upload does not enter lifecycle admission.
12. **P6.3 verified:** real loopback graceful shutdown rejects live-handler
    new work as `503 unavailable`, closes listener acceptance, drains accepted
    active and queued work in order, joins the worker, cleans workspaces, and
    reaches `Stopped`; repeated shutdown is safe.
13. Invalid credentials return `401` before admission.
14. **P6.4 verified:** an already admitted request that exceeds the existing
    post-dequeue execution deadline returns `504`, completes its cleanup, and
    is still drained by graceful shutdown; expiry does not cancel native work.
14. If `Idempotency-Key` is accepted, prove active-window behavior; do not emulate durable replay.

## P5 Status

**IMPLEMENTED + VERIFIED:** the narrow first vertical slice is implemented and
verified in the Ubuntu 24.04 Docker gate. This is not a production-readiness
claim or a replacement for the legacy API.

**PRODUCTION STATUS: NOT READY / NOT CLAIMED.**

**LEGACY API: UNCHANGED + SUPPORTED.** `/api/generate` remains the supported
legacy API until a separate parity, migration, rollback, and retirement
decision.

### Authentication

`X-API-Key` is required before multipart body admission or native processing.
The Rust service reads `IMGENGINE_API_KEYS` from process configuration. Its
value is a non-empty comma-separated list of non-empty API-key values; leading
and trailing ASCII whitespace around list entries is ignored. No database,
job state, or other persisted authentication data is used.

Startup fails closed when `IMGENGINE_API_KEYS` is missing, empty after parsing,
or contains an empty entry. The service must not start a route that accepts
requests when authentication configuration is invalid. Credentials are compared
against every configured active key using a constant-time equality operation;
the configuration supports multiple simultaneously active keys. Rotation is
performed by configuring old and new keys together, deploying the change, then
removing the old key after clients migrate.

Tests supply an explicit test-only process configuration value or an equivalent
explicit configuration object; test keys must be non-production values and must
not be committed as secrets. A missing key and an invalid key both return the
same generic `401 authentication_failed` response and reveal neither the
configured key set nor the reason for failure. API keys, raw authentication
headers, and derived credential values are never logged, traced, returned, or
stored.

### First capability

The first HTTP exposure is `POST /api/v1/render`, accepting multipart field
`file` only. It supports JPEG and PNG input and returns a completed JPEG
response synchronously. It is ephemeral: it creates no job identifier, polling
endpoint, durable artifact, database record, queue record, or permanent image
metadata.

This is the first **capability-backed HTTP contract**, not an assertion that
the broader planned API exists. The endpoint adapts the existing
JPEG/PNG-to-JPEG lifecycle and ABI path; it does not implement a layout, preset,
PDF, job, polling, or durable-artifact capability.

### Deferred capabilities

Layout fields, presets, PDF output, and broader output capabilities remain
planned roadmap items. The older broad target in `docs/api.md` is retained as
a future target; it does not describe the first Axum slice. HTTP must expose a
capability only after the corresponding engine, ABI, and lifecycle behavior is
implemented and verified.

### Rate limiting and admission

Bounded request/body limits and bounded admission/concurrency are mandatory for
the first slice. Admission answers whether a request can enter the bounded
processing system and maps a full queue to `429 overloaded` without retaining
unbounded image bytes.

**Implemented provisional integration values:** the adapter defaults to a
1 MiB HTTP/admission input limit, queue capacity 1, and a 10-second lifecycle
deadline. These values keep the first slice bounded and testable; they are not
production capacity commitments. The HTTP cap is applied by Axum before
multipart extraction, and the same byte bound is passed to lifecycle admission.

Client-frequency rate limiting is a separate concern and is deferred. This
slice adds no Redis, PostgreSQL, Celery, database-backed counter, or distributed
rate-limit dependency. Any later in-process rate policy requires separate,
bounded design and verification; it must use a response distinct from admission
overload.

### P6.4 resource and deadline evidence

| Limit / budget | Current owner and behavior | Status |
| --- | --- | --- |
| HTTP request body | Axum `DefaultBodyLimit` rejects bodies beyond the configured 1 MiB integration value before multipart retention. | IMPLEMENTED + TESTED; PROVISIONAL |
| Lifecycle / admission input | `RequestPolicy` and `AdmissionController` apply the stricter 1 MiB configured bound to owned image bytes; boundary rejections map to `413`. | IMPLEMENTED + TESTED; PROVISIONAL |
| Decoded dimensions / pixels | Native validation rejects zero dimensions, either dimension above 16,384, more than 268,435,456 pixels, implausible compression ratios, and RGBA estimates above 4 GiB before unsafe processing. | IMPLEMENTED + TESTED native safety guard; product-level limit DEFERRED |
| JPEG output bytes | The safe wrapper returns a Rust-owned in-memory `Vec<u8>` after native encode. There is no configured post-encode output-byte ceiling. | NOT IMPLEMENTED |
| Workspace / temporary disk bytes | Workspaces are private, ephemeral directories and are removed on completion/failure/deadline/abandonment; no per-workspace or aggregate byte accounting exists. | IMPLEMENTED + TESTED cleanup; byte limit NOT IMPLEMENTED |
| Queue capacity | One bounded Rust admission slot plus one active worker; overflow is `429 overloaded`. | IMPLEMENTED + TESTED; PROVISIONAL |
| Worker / concurrency | One `Supervisor` worker owns the one ABI-v1 engine. Rust admission is control-plane only; C remains the scheduler/execution plane. | IMPLEMENTED + TESTED; PROVISIONAL characterization only |
| Execution deadline | The provisional 10-second duration begins after dequeue, immediately before `Supervisor::process`; it is checked after native execution and workspace cleanup. A late result is discarded as `504 deadline_exceeded`. | IMPLEMENTED + TESTED; PROVISIONAL |
| Queue-wait budget | Queue waiting is intentionally outside the execution deadline and has no separately configured budget. | IMPLEMENTED semantics; separate budget DEFERRED |
| HTTP response / transport budget | Client disconnect abandons response delivery; no server response timeout or write-delivery budget exists. | DEFERRED |
| Graceful-drain budget | P6.3 drains accepted work with no internal maximum duration; deployment termination remains external. | IMPLEMENTED drain semantics; budget DEFERRED |
| Startup stale-workspace cleanup | Supervisor startup removes stale `request-*` directories older than five minutes, with symlink-safe handling. | IMPLEMENTED + TESTED; PROVISIONAL operational retention |

The native pixel guard is exercised by hostile-dimension security regressions
and sanitizer/fuzz gates. P6.4 does not select a lower product pixel ceiling:
that requires measured memory/RSS and representative-image evidence. Likewise,
an output ceiling can only protect retained response memory after native encode
with the current ABI; it cannot prevent native allocation without a separately
designed native/ABI capability. Workspace accounting and periodic sweeping are
not present; startup sweep and per-request cleanup are the only current disk
controls.

Queue time, multipart parsing, and HTTP response delivery do not consume the
10-second execution duration. It covers post-dequeue `Supervisor::process`,
native execution, and workspace cleanup. ABI v1 means deadline expiry and
client disconnect both allow queued/active native work to finish; only late
output or response delivery is discarded. Graceful drain waits for that same
accepted work, subject to an external deployment termination budget.
The P6.4 loopback test explicitly covers an accepted request that reaches this
deadline while drain is in progress: it returns `504`, is cleaned up, and the
worker is joined without attempting native cancellation.

## Verification Environment — Ubuntu 24.04 Docker Gate

Development and Linux verification are distinct:

| Role | Environment |
| --- | --- |
| Development host | Windows 11 with VS Code |
| Verification mechanism | Docker Desktop on the Windows host |
| Authoritative Linux target | Ubuntu 24.04 Docker container |

Docker Desktop is the development/verification mechanism; Ubuntu 24.04 is the
authoritative environment for Linux-native integration evidence. A future Axum
implementation is not fully verified merely because a Windows-native build or
unit-test run passes.

```text
Windows 11 host
      -> Docker Desktop
      -> Ubuntu 24.04 Linux container
      -> Rust + Axum
      -> RequestLifecycleService
      -> bounded admission
      -> safe Rust FFI
      -> libimgengine C ABI v1
      -> C scheduler
      -> C native engine
```

### CURRENT VERIFIED

Existing native, ABI v1, safe Rust FFI, supervisor, bounded-admission, and
transport-neutral lifecycle evidence is documented separately and uses the
Ubuntu 24.04 Docker native verification gate where applicable. That evidence
does not establish HTTP/Axum behavior.

The P5 adapter's focused in-process Axum integration tests have passed in this
Ubuntu Docker gate: authentication, multipart policy, JPEG/PNG processing,
safe error mapping, provisional limits, admission overload, deadline
classification, independently decoded JPEG responses, and workspace cleanup.
This is Linux evidence for the in-process Axum-router-to-native path. Separate
P6.2 and P6.3 loopback tests establish the implemented socket-disconnect and
graceful-drain behaviors; none of this establishes production capacity.

### FUTURE AXUM GATE REQUIREMENTS

Before calling a future HTTP adapter verified, run the applicable checks from a
clean Ubuntu 24.04 container:

1. Build the native and Rust/Axum components from a clean environment.
2. Run the existing native C test suite, Rust unit/integration tests, and ABI/FFI verification.
3. Exercise a project-owned real JPEG through HTTP -> Axum -> lifecycle -> admission -> safe FFI -> C scheduler -> C engine -> HTTP response.
4. Exercise a real PNG when supported by the lifecycle/API contract.
5. Test malformed image input, HTTP body/input limits, and admission overload.
6. Verify cleanup after success and failure, response abandonment/client-disconnect semantics, deadline behavior, and graceful shutdown/draining.
7. Run sanitizer/native verification and preserve sandbox/security verification wherever the existing native gate requires them.
8. Run `git diff --check`.

These were future Axum/P5.x gate requirements. P6.2/P6.3 now provide loopback
TCP disconnect and graceful-drain evidence; this remains distinct from
production-capacity or deployment termination evidence.

### NOT YET VERIFIED

Production capacity, deployment/process termination budgets, and response-write
delivery confirmation are not yet verified. P6.2 establishes loopback TCP
disconnect evidence for the admitted response path and pre-admission upload
disconnect. P6.3 establishes real loopback graceful drain, but does not turn a
connection after listener closure into an HTTP-status guarantee.

Pre-lifecycle HTTP errors currently use sequential transport request IDs;
admitted requests use entropy-backed lifecycle IDs. This is a non-blocking
future hardening item and does not change P5 authorization or processing.

### Reproducible evidence

An accepted Axum change must record the Ubuntu version; compiler, Rust,
CMake/Ninja versions where relevant; commands; test results; commit; real-image
fixture identity/provenance; request/result classification; independent output
validation; and cleanup verification. Use deterministic project-owned fixtures
or documented licensed fixtures; do not require personal or private images.

## Adapter boundary

| Layer | Responsibilities |
| --- | --- |
| Axum | Authentication, rate/body limits, multipart parsing, HTTP mapping, headers, socket observation, HTTP metrics/tracing |
| Lifecycle | ID, validation, admission, deadline, state, workspace/process orchestration, cleanup, redacted classification |
| C | Scheduler, decoding, native execution, JPEG encoding, memory, sandbox |

Axum must not create engines/workspaces, use raw FFI, duplicate lifecycle
validation, or implement scheduler/lifecycle state machines.

## Future P5.x Gate

P6.2/P6.3 provide the required real HTTP disconnect and graceful-shutdown
evidence without changing the established P5 contract: process-configured
`X-API-Key` authentication, multipart `file` only, JPEG/PNG-to-JPEG, bounded
admission, and no distributed rate limiting. This does not authorize a
production release, API parity claim, or legacy retirement.

Open implementation/release questions are production size/disk/output/deadline/
drain values; active-window idempotency policy; capability-backed layout/PDF
work; future rate limiting; production capacity; response-write delivery; and
deployment termination evidence. They must be addressed by separate design,
tests, and future release evidence without weakening this contract.

## Smallest implementation slice

Authenticated `POST /api/v1/render` with multipart `file` only, bounded body
extraction, direct `RequestLifecycleService` adaptation, completed-buffer JPEG
response, stable problem responses, real ABI v1 execution, real cleanup, and
the cases above. It adds no job storage, Redis, PostgreSQL, Celery, layout/PDF,
or native cancellation.
