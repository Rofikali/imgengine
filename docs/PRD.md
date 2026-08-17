# IMGENGINE Product Requirements Document

**Status:** Living document  
**Owner:** Product and Engineering  
**Last updated:** 2026-08-17
**Delivery model:** Thin vertical slices; every loop ends with an observable user outcome and automated evidence.

## 1. Product Summary

IMGENGINE is a SaaS for print shops, studios, and automation teams that turns a source JPEG or PNG into a print-ready photo sheet. It combines a deterministic native image engine with an asynchronous web product so a user can upload an image, choose a layout, follow the job state, and download a reliable output.

The product is not a general-purpose image editor. Its differentiators are predictable layout, physical print dimensions, professional cut safety, and an automation-ready API.

## 2. Problem and Opportunity

Print operators repeatedly create passport, ID, and studio sheets by hand. Existing workflows are slow, inconsistent, and make it easy to produce an incorrectly sized or unsafe-to-cut output. API consumers also need a dependable image-processing primitive without owning native image-pipeline operations.

IMGENGINE reduces this work to a validated job while preserving reproducible layout parameters, job history, and a clear failure state.

## 3. Target Users and Jobs To Be Done

| User | Job to be done | Success signal |
| --- | --- | --- |
| Print-shop operator | Produce a compliant sheet quickly from one photo. | Receives a print-ready file without manual layout work. |
| Studio operator | Reuse a known layout across customer jobs. | Preset output is consistent across operators. |
| Automation developer | Submit jobs programmatically and retrieve results. | A stable API, status lifecycle, and deterministic output. |
| Platform operator | Run the service safely and diagnose failures. | Measurable queue, worker, engine, and storage health. |

## 4. Product Principles

1. **Correctness before optimization.** A valid, reproducible output beats an unverified performance claim.
2. **Separate control and execution.** Web/API code orchestrates jobs; the native engine processes pixels.
3. **Asynchronous by default.** Upload acceptance must not wait for CPU-bound processing.
4. **Failure is a product state.** Every accepted job ends in `completed`, `failed`, or an explicit retry policy; no silent loss.
5. **Measured performance.** The native RFC target of `<2 ms` applies only to its defined render-only benchmark, never to end-to-end SaaS latency.
6. **Secure boundaries.** Validate uploads before persistence, keep internal control endpoints private, and never expose worker credentials to browsers.

## 5. Launch Scope

### In scope

- JPEG and PNG upload.
- Configurable grid: rows, columns, gap, padding, dimensions, DPI, border, bleed, and crop-mark settings.
- Asynchronous job submission, status polling, and output download.
- JPEG output for the first externally usable release; PDF follows after validated engine support.
- API-key protected public API and worker-only internal status updates.
- Local development with Compose; production configuration through environment variables.
- Structured logs, metrics, traces, and health/readiness endpoints.

### Explicitly out of scope for launch

- General-purpose image editing, collaborative editing, and a desktop GUI.
- GPU-first processing.
- Billing, self-service accounts, and multi-tenant administration.
- S3/object storage until local storage has lifecycle and cleanup guarantees.

## 6. Primary User Flow

1. User selects a JPEG or PNG in the web app.
2. Nuxt sends the form to its server-side proxy; the browser never receives the upstream API key.
3. API validates content type, size, filename, and layout settings; it persists a `queued` job and stores the source file.
4. API verifies the queue is reachable, then publishes a versioned job payload.
5. Worker transitions the job to `processing`, invokes a pinned native-engine artifact, and records structured execution output.
6. Worker stores the result, transitions to `completed` or `failed`, and emits metrics/traces.
7. UI polls status, displays a human-readable failure if applicable, and offers a download only for a completed job.

## 7. Functional Requirements

| ID | Requirement | Acceptance criteria |
| --- | --- | --- |
| FR-01 | Accept supported image uploads. | JPEG/PNG accepted; unsupported types return `415`; configured size limit returns `413`; traversal filenames cannot escape upload storage. |
| FR-02 | Validate layout requests. | Rows, columns, dimensions, DPI, and output format have bounded schema validation before queueing. |
| FR-03 | Create durable jobs. | Each accepted request receives a UUID, immutable request parameters, timestamps, and a status. |
| FR-04 | Publish work safely. | Queue unavailability returns `503` within three seconds and records a failed submission reason; no `500` is exposed for this condition. |
| FR-05 | Execute jobs asynchronously. | A healthy worker changes `queued → processing → completed|failed`; every transition is auditable. |
| FR-06 | Retrieve output safely. | Completed outputs are downloadable through an authorized endpoint; filesystem paths are never the public contract. |
| FR-07 | Provide browser workflow. | UI submits an image, shows current status, surfaces API failure detail, and only enables output download when complete. |
| FR-08 | Preserve API compatibility. | OpenAPI contract is versioned; breaking changes require a new API version and migration note. |

## 8. Non-Functional Requirements

| Area | Requirement |
| --- | --- |
| Availability | API liveness and readiness are separate; readiness fails when mandatory dependencies are unavailable. |
| Latency | API upload acknowledgement p95 `<1 s` excluding client upload time when dependencies are healthy. Queue failure response `<3 s`. End-to-end targets are defined only after benchmark baselines exist. |
| Reliability | Jobs are idempotent by idempotency key or caller-provided request key; retries cannot produce duplicate billable outputs. |
| Security | Size/type validation, secret-only server-side API keys, authenticated internal worker route, least-privilege volumes, and no default production secrets. |
| Observability | Request/job correlation ID, structured logs, queue/worker/job metrics, traces, and alertable dependency health. |
| Native performance | Zero allocation in defined hot paths, scalar fallback, ABI checks, sanitizers, and benchmark regression gates as stated in the engine RFC/HLD. |
| Portability | Linux is the production runtime. Windows developer support requires a tested file-mapping abstraction and does not claim Linux-only features such as seccomp/io_uring. |

## 9. Data and Lifecycle

- Source uploads and outputs are separate, private storage classes.
- Jobs retain normalized parameters, status, timestamps, trace ID, error code/message, and engine artifact version.
- Define a configurable retention policy before external release; a deletion worker removes expired uploads and outputs.
- Do not log source image bytes, API keys, or unredacted customer filenames.

## 10. Architecture Contract

- **Nuxt:** user-facing control surface and server-side API proxy.
- **FastAPI:** validation, job persistence, authorization, and job publication.
- **Redis/Celery:** broker and background delivery mechanism for the initial release.
- **Worker:** state transitions, engine process supervision, resource limits, and output persistence.
- **Native engine:** deterministic decode/layout/render/encode pipeline with scalar/SIMD dispatch.
- **PostgreSQL:** source of truth for job metadata in deployed environments. SQLite is development-only.

The API-to-worker payload is versioned. The worker must reject unknown payload versions and invalid state transitions.

## 11. Current Readiness Assessment

| Area | State | Required before usable release |
| --- | --- | --- |
| Nuxt upload flow | Built and running locally. | Add output download and richer job details. |
| API upload boundary | Validates files and returns fast `503` when Redis is unavailable. | Add schema bounds, readiness check, and integration tests. |
| Queue/worker | Skeleton exists. | Run Redis + worker end-to-end, add timeouts/idempotency/retry policy. |
| Native engine | CMake dependency integration works. | Fix Windows `mmap` abstraction or declare Linux-only dev support; build and test CLI artifact. |
| Container deployment | Compose describes services. | Build native artifact inside the worker image and verify a clean `compose up --build`. |
| Production operations | Metrics/tracing files exist. | Define dashboards, alerts, migrations, backups, retention, and secret management. |

## 12. Delivery Loops

Work one loop at a time. Do not begin the next loop until its exit criteria and automated evidence are met.

### Loop 0 — Baseline and Truth

**Outcome:** a reproducible development environment with no unsupported claims.  
**Exit criteria:** native build platform documented; engine test command runs; API/Nuxt build commands pass; Compose stack has a single documented start path.

### Loop 1 — Vertical Slice

**Outcome:** one uploaded image becomes a downloadable PNG through the real queue and worker.  
**Exit criteria:** integration test proves `queued → processing → completed`; output is valid PNG; failed engine invocation becomes `failed`; queue outage returns `503` within three seconds.

### Loop 2 — Operational Reliability

**Outcome:** an operator can detect, retry, and recover a failed job.  
**Exit criteria:** readiness probes, bounded worker subprocesses, idempotency, retry/DLQ policy, job cleanup, and a dashboard for job/queue/worker health.

### Loop 3 — Product Fit

**Outcome:** print operators use stable presets and retrieve outputs confidently.  
**Exit criteria:** passport/studio presets, parameter validation UX, download endpoint, output expiry messaging, and a representative acceptance suite.

### Loop 4 — Commercial Scale

**Outcome:** controlled multi-user usage and durable storage.  
**Exit criteria:** tenant-aware auth, quotas, object storage, audit trail, billing decision, and load-tested worker scaling.

## 13. Ordered Backlog

| Priority | Item | Loop | Definition of done |
| --- | --- | --- | --- |
| P0 | Make the native CLI build reproducibly on the declared development platform. | 0 | Clean build plus CLI smoke test and CTest pass. |
| P0 | Build/copy the native CLI inside the worker image; remove dependence on an untracked binary. | 0/1 | `docker compose up --build` produces a worker containing a verified CLI. |
| P0 | Maintain API/worker integration tests with Redis and PostgreSQL. | 1 | CI proves JPEG, PNG, progressive JPEG, and CMYK JPEG jobs reach terminal state with artifact, idempotency, timeline, and log assertions. |
| P0 | Add `/readyz`, dependency checks, and documented local bootstrap. | 0/2 | Health semantics are tested; queue outage is actionable. |
| P1 | Add authenticated output-download endpoint and UI action. | 3 | Completed job is downloadable; non-completed job returns correct status. |
| P1 | Version the job payload and enforce job transition state machine. | 1/2 | Invalid version/transition is rejected and recorded. |
| P1 | Add migration tooling and remove runtime `create_all` from production startup. | 2 | Fresh and upgraded PostgreSQL deployments are deterministic. |
| P1 | Establish native sanitizer, ABI, regression, and benchmark CI gates. | 0/2 | CI publishes measured results and blocks regressions. |
| P2 | Add presets, batch submissions, retention cleanup, and object-storage abstraction. | 3/4 | Each feature has end-to-end acceptance coverage. |
| P2 | Add accounts, tenants, quotas, and billing only after usage evidence. | 4 | Authorization and usage data are isolated per tenant. |

## 14. Risks and Decisions Needed

1. **Native platform:** choose Linux-only development for the first release or fund/complete the Windows memory-mapping implementation.
2. **Performance claim:** establish a reproducible benchmark corpus before advertising RFC targets.
3. **Output formats:** verify PDF encoder quality and color management before offering PDF commercially.
4. **Compliance:** define passport-country rules as versioned presets rather than hard-coded assumptions.
5. **Storage:** decide retention, backup, deletion, and data-residency requirements before handling customer production data.

## 15. Definition of Release Readiness

An external beta is ready only when Loop 1 and Loop 2 are complete, the native artifact is reproducibly built in CI, the worker executes a real job in a clean environment, secrets are externalized, and on-call operators can identify and recover from queue, worker, storage, and engine failures.

## 16. Production Engineering Gates

- The release gate proves JPEG, PNG, progressive JPEG, and CMYK JPEG processing, idempotency, audit events, durable service logs, and artifact integrity.
- SLOs and Prometheus alert rules are defined in the SRE contract; local file logs remain a recovery aid while JSON stdout is the production collection path.
- Current API-key ownership is sufficient for controlled beta use. Tenant principals, quota ledgers, key rotation, and malware-scanning policy are mandatory before commercial multi-tenant launch.
