# IMGENGINE Principal Audit — 2026-10-31

**Audit type:** Initial repository-wide architecture, security, product, operations and commercial-readiness review  
**Baseline:** `main` as observed on 2026-10-31  
**Repository:** `Rofikali/imgengine`  
**Status:** Initial audit; source-level and deployment-level verification remain open  
**Important:** This document is not a penetration-test report, legal opinion, financial forecast, or production-readiness certification.

## 1. Executive summary

IMGENGINE has a credible systems-engineering foundation: a C11 native processing engine, a deliberately narrow/versioned C ABI, a safe Rust FFI wrapper, Rust lifecycle/admission boundaries, an Axum adapter, a Nuxt frontend, native regression/sanitizer/fuzz gates, and explicit product contracts.

The strongest architectural choice is to retain C for proven codec/render/SIMD/native scheduling work while Rust owns HTTP, request policy, admission, lifecycle, filesystem handling and observability. Do not migrate the C scheduler or allocator merely to make the codebase more Rust-centric.

The project is **not yet proven production-ready**. The current audit identifies unresolved or insufficiently evidenced areas: encoded output and aggregate memory/disk ceilings, cancellation semantics, hard process isolation, real-VPS capacity, authenticated tenant lifecycle, API contract drift, production packaging, deployment evidence, licensing provenance, and commercial unit economics.

## 2. Repository and GitHub facts

Observed through the connected GitHub repository on 2026-10-31:

- Public repository: `Rofikali/imgengine`; default branch `main`.
- GitHub reports the repository as a fork of `ramdasbhumi-ai/imgengine`.
- Connected account permissions reported by GitHub: admin, maintain, push, triage and pull.
- The repository metadata reports `license: null`; fetching root `LICENSE` returned 404, while README advertises MIT. This discrepancy must be resolved before commercial distribution.
- Latest observed push: 2026-10-31.
- Open PRs observed: #13 (P11 geometry planner) and #14 (P12 physical-sheet contract); PR #12 (P10 contract refresh) is merged.
- Current active branch list includes P8 resource-audit, P10 documentation, P11 geometry, P12 sheet-contract and main branches.

Repository access does not imply access to organization billing, secrets, deployment environments or every account-level administration endpoint.

## 3. Architecture assessment

### Intended boundary

```text
Nuxt / Vue / TypeScript
        |
        v
Rust / Axum HTTP adapter
        |
        v
Rust request lifecycle + bounded admission
        |
        v
Safe Rust FFI wrapper
        |
        v
Versioned C ABI
        |
        v
C scheduler + C11 engine
        |
        +-- decode / validate
        +-- geometry / crop / resize
        +-- render / encode
        +-- SIMD and native memory
```

Ownership recommendation:

- Nuxt: user experience, typed API client, preview and download flow. No secrets or authoritative validation.
- Rust: HTTP, authentication/authorization, validation, admission, request lifecycle, temporary workspace policy, deadlines, telemetry and safe error mapping.
- C: authoritative native validation, decode, geometry safety checks, rendering, codec calls, native scheduling and native memory.
- Edge/reverse proxy: TLS, request ingress controls and network boundary.
- No database, Redis, object storage or distributed queue until a measured product requirement justifies them.

### Positive decisions

1. Keep the native C engine and its performance-proven paths.
2. Keep raw FFI operations inside a small Rust wrapper; do not make the opaque engine `Send` or `Sync` without proof.
3. Retain C scheduler ownership until comparative measurements justify a separate change.
4. Keep the old FIT endpoint behavior stable; introduce crop-to-fill as a separately versioned operation.
5. Treat browser validation as convenience only; independently validate in Rust and C.
6. Keep uploaded bytes and generated outputs ephemeral by default.

### Important implementation caveat

The repository contains both a legacy FastAPI/Celery/Redis/PostgreSQL application and newer Rust/Axum components. Documentation must clearly distinguish the currently deployed path, verified Rust paths, legacy paths and proposed target paths. Do not infer a complete migration from the presence of Rust crates.

## 4. Current product and API position

### P8

The documented P8 operation is a synchronous, ephemeral FIT-only photo layout endpoint:

`POST /api/v1/layout-render`

Its width/height represent individual photo-cell dimensions, not the physical sheet. It has bounded input/plan rules and JPEG output. The P8 audit records a 1 MiB provisional request limit, one active plus one waiting admission slot, and a provisional 10-second post-dequeue deadline.

### P9.3

The bounded-output design adds a server-owned JPEG destination allocation budget (documented provisional default 16 MiB). This bounds the encoded destination allocation, not all decoder, canvas, intermediate, Rust-copy or aggregate process memory.

### P10

The product contract freezes the MVP around one JPEG/PNG source, one custom-dimension JPEG sheet, mm-only customer measurements, centered crop-to-fill, exact copy count or rejection, regular row-major grid and ephemeral processing. The 300 DPI UI default is editable and is not a print-quality guarantee.

### P11 / P12

- P11 is a pure Rust geometry/crop planner and must not decode images, allocate raster buffers or call C.
- P12 proposes a separate `POST /api/v2/sheet-render` operation and additive ABI 2.2.
- The existing P8 FIT endpoint must not silently change to crop-to-fill.
- C must independently validate crop, canvas, placement, count, reserved fields and resource bounds. Rust-computed geometry is not trusted merely because Rust generated it.

P12 is explicitly a design/documentation gate, not evidence that its endpoint, ABI or native renderer already exists.

## 5. Security risk register

| ID | Priority | Finding / risk | Required evidence or action |
|---|---|---|---|
| SEC-01 | P0 | License/provenance discrepancy: forked repository, README MIT badge, GitHub license null, root LICENSE not found | Inspect upstream license and all vendored/codec licenses; add correct license and attribution; obtain legal review if needed |
| SEC-02 | P1 | Encoded output cap does not bound total memory | Define and test decoded pixels, canvas, intermediates, encoded output, Rust copies, aggregate RSS and per-request memory budget |
| SEC-03 | P1 | Temporary disk and aggregate workspace accounting need proof | Enforce per-request and process-wide byte quotas; test concurrent requests, cleanup failures and disk-full behavior |
| SEC-04 | P1 | ABI v1 has no mid-operation cancellation | Explicitly document that disconnect/deadline does not stop native work; prove bounded admission, worker draining and overload behavior |
| SEC-05 | P1 | Native decoder remains a high-risk parser boundary | Verify non-root execution, no network, seccomp/no_new_privs, process/resource limits, filesystem isolation and forced-kill cleanup |
| SEC-06 | P1 | API authentication/tenant lifecycle is incomplete for a commercial multi-tenant product | Define tenant principal, session/API-key lifecycle, rotation, revocation, quota accounting, abuse controls and ownership tests |
| SEC-07 | P1 | Request and response limits need end-to-end verification | Enforce proxy, Axum, multipart, decoder, renderer and response limits; reject unknown/duplicate fields |
| SEC-08 | P1 | Secrets and browser boundary | Verify Nuxt private runtime config, no secrets in public bundle/HTML/logs, secret-file permissions and rotation |
| SEC-09 | P1 | Supply-chain integrity not yet demonstrated end-to-end | Pin image digests/actions/dependencies; generate SBOM; scan; record provenance and immutable deployment digest |
| SEC-10 | P2 | Logs/metrics may accidentally reveal personal image data | Allow-list fields; redact filenames, paths, bytes, credentials and image metadata; add automated redaction tests |
| SEC-11 | P2 | Symlink/path traversal and cleanup under hostile filesystem state | Use generated paths and safe create semantics; test symlink races, permissions, stale sweep and abrupt process death |
| SEC-12 | P2 | Rate limiting and quotas need tenant/IP fairness | Add per-principal and edge limits; test bursts, queue saturation and retry abuse |

### Required hostile-input matrix

At minimum: malformed/truncated JPEG and PNG; MIME spoofing; huge dimensions; decompression bombs; integer overflow; NaN/infinity/negative geometry; zero-after-rounding dimensions; excessive copies; invalid/overlapping/out-of-canvas placements; unknown crop modes; non-zero reserved fields; short structures; null pointers with non-zero lengths; output-budget boundary; allocation failure; disk full; symlink race; queue full; request timeout; disconnect before and after admission; worker crash; graceful shutdown; forced termination; concurrent cross-request isolation.

## 6. Reliability, performance and capacity

The P6.5 report is a characterization run on Docker Desktop/WSL2, not dedicated VPS capacity evidence. It reports a one-worker/one-waiting-slot configuration and observed overload rejections under concurrency. Treat those numbers as a reproducible historical sample, not an SLA or customer capacity promise.

Required next evidence:

1. Dedicated Ubuntu 24.04 VPS smoke test with exact machine, kernel, cgroup, memory, disk and Docker versions.
2. Cold-start and warm throughput, p50/p95/p99 latency, CPU, peak RSS, temporary disk and output-size distributions.
3. Mixed small/large JPEG and PNG workloads, malformed traffic and sustained load.
4. Queue saturation and recovery; no unbounded buffering.
5. Graceful shutdown with in-flight requests and deployment termination budget.
6. Real client disconnect tests before admission, while queued, during native work and during response delivery.
7. Health/readiness/liveness semantics that do not require an expensive render.
8. Restore/rollback and deployment verification.

Do not advertise a throughput, concurrency or VPS-size guarantee until it is supported by repeatable measurements with headroom.

## 7. CI and pull-request findings observed on 2026-10-31

- P11 PR #13 run `41160976022`: observed successful jobs, including Ubuntu 24.04 Docker verification, native build/test, sanitizers, fuzzing and legacy SaaS integration.
- P12 PR #14 run `41171854866`: overall failure. Native sanitizer and fuzz jobs passed; SaaS integration passed; `native-build-test` failed at `Run format check`; Ubuntu 24.04 Docker verification failed at its aggregate verification step.
- The exact failing formatter output was not available in the retrieved job summary. The immediate action is to inspect the job log, correct the format failure, rerun the complete verifier and confirm all checks green.
- Do not merge P12 while its required CI is red.
- PR #14 targets P11 branch rather than main, making it a stacked PR. Preserve and explain this dependency; merge in order and rebase/retarget only deliberately.

CI is valuable evidence, but passing CI does not equal a security audit or production approval.

## 8. Documentation consistency findings

The repository has extensive useful documentation, but several documents describe different lifecycle stages:

- Root README describes the C + Rust target, while repository metadata description still says FastAPI.
- `docs/architecture.md` labels FastAPI/Celery/Redis/PostgreSQL as the current architecture and Rust as target.
- `docs/SAAS_CONTRACT.md`, `docs/SRE.md` and `docs/SECURITY_OPERATIONS.md` contain legacy job/queue/database contracts.
- Frontend AGENTS guidance says the current Rust API is only `POST /api/v1/render`, while P8 documents a later layout endpoint.
- Backend AGENTS guidance describes the narrow Rust adapter and planned bounded response streaming.
- `docs/RUST_SUPERVISOR.md` states the P5 path holds complete input/output bytes in memory and lacks an output-byte ceiling in that path.
- P8/P9 documents describe later bounded layout output, so these claims must be scoped by endpoint and phase, not treated as one universal API state.

Recommended source-of-truth hierarchy:

1. Versioned public ABI/API contract for each endpoint.
2. Current implementation and tests at the audited commit.
3. Phase-specific evidence report.
4. Architecture decision records.
5. README and general AGENTS guidance.

Add a compact status matrix: implemented + tested; implemented but not production-qualified; legacy; planned; retired.

## 9. Product, customer and commercial assessment

### Initial positioning

The narrow MVP should solve one paid customer problem extremely well: produce correctly sized, print-ready photo sheets from one uploaded image with predictable copy count, layout and download.

Potential first customer segments to validate: photo studios, passport/ID photo agents, print shops and small document-service shops.

### Product discipline

Do not expand into a general editor, template marketplace, multi-image collage, PDF suite, accounts, teams or batch API until customer evidence justifies the cost.

Validate willingness to pay with real workflows:
- time saved per customer order;
- rejected/reprinted sheets avoided;
- print-size accuracy and output acceptance;
- repeat usage per studio/agent;
- support incidents per 100 renders;
- paid conversion and repeat purchase.

### Unit economics — required model

For each successful paid render, calculate:

`Contribution = Net revenue - payment fees - variable compute - bandwidth - storage/retention - support/refund cost`

Then calculate monthly break-even:

`Break-even paid renders = Monthly fixed costs / Contribution per paid render`

Include failed renders, free usage, payment gateway fees, taxes, refunds, support, VPS upgrades, domain, monitoring, backups and customer acquisition. A low per-image price is not automatically profitable; the full cost per successful paid output must be measured. Do not confuse revenue, cash received and accounting profit.

No forecast is certified in this audit because current vendor prices, actual workload, tax treatment, conversion and customer demand have not been measured.

## 10. Management and release governance

Every work item should have:
- one user-visible or engineering outcome;
- written contract and non-goals;
- threat/risk analysis;
- tests added before or with the fix;
- exact verification commands and environment;
- before/after evidence;
- rollback path;
- owner and acceptance criteria.

Release gates should be independent:
1. Native correctness and ABI gate.
2. Rust API/lifecycle gate.
3. Frontend contract/build/accessibility gate.
4. Security/resource-isolation gate.
5. Capacity/observability gate.
6. Product acceptance gate.
7. Commercial/legal release gate.

A release candidate is not a production release. Publishing artifacts, deploying, migrating data and changing traffic require separate approval.

## 11. Prioritized roadmap

### P0 — Establish trustworthy baseline
- Resolve license/fork provenance and add authoritative license/attribution.
- Fix P12 PR CI formatting/aggregate verifier failure.
- Record exact main SHA and PR dependency graph.
- Publish implementation/legacy/planned status matrix.
- Reconcile README, repository description, architecture docs and AGENTS files.

### P1 — Complete P11/P12 safely
- Review P11 geometry edge cases and property tests.
- Review P12 ABI 2.2 structure sizing, alignment, reserved fields, ownership, count semantics and symbol versioning.
- Implement only after contract approval.
- Add C-side independent validation and adversarial tests.
- Preserve P8 endpoint compatibility.
- Require green full Ubuntu verifier.

### P2 — Resource and lifecycle hardening
- End-to-end input/output/memory/disk/concurrency budgets.
- Bounded response delivery or a documented safe bounded in-memory response design.
- Disconnect, deadline, graceful shutdown and forced-kill tests.
- Dedicated VPS capacity characterization.
- Health/readiness and operational alert evidence.

### P3 — Product and commercial validation
- Run pilot with a small set of real studios/agents.
- Measure print correctness, time saved, repeat usage and support burden.
- Establish contribution margin and break-even.
- Introduce billing only after usage metering and idempotency are correct.

### P4 — Production release
- Tenant identity, quotas, API-key/session lifecycle and abuse protection.
- Immutable images, SBOM, vulnerability scanning and deployment provenance.
- Backups only for data that the product actually retains; test restore.
- Incident response, privacy/retention policy, terms, support and rollback.
- Controlled beta followed by staged rollout.

## 12. Audit limitations and next inspection

This initial audit used GitHub repository metadata, directory listings, selected documentation, PR metadata and Actions job summaries. It did not yet inspect every C/Rust/TypeScript source file, all workflow logs, deployment host configuration, live production traffic, secrets, dependency advisories, or customer economics.

Therefore:
- Absence of a finding is not proof of absence of a vulnerability.
- A documented control is not proof it is deployed.
- A passing test is evidence only for the tested conditions.
- Production readiness remains unclaimed.

The next source-level audit should proceed in this order:
1. Public C headers, ABI version scripts, export checker and allocation/release paths.
2. Decoder, dimension validation, geometry, crop/render, encoder and output budget.
3. Scheduler, worker lifecycle, allocator/arena/slab and shutdown.
4. Rust unsafe FFI, safe wrapper, supervisor, admission and lifecycle.
5. Axum multipart/auth/error/response handlers and config.
6. Nuxt server proxy, runtime config, API client, upload/download flow and tests.
7. Dockerfiles, Compose profiles, reverse proxy, deployment scripts and release workflows.
8. Dependency/licensing inventory and full commercial unit-economics model.

## 13. Decision

**Current recommendation: continue development; do not declare production-ready or publicly launch paid workloads yet.**

The architecture is directionally strong. The next best move is not adding infrastructure or rewriting the scheduler. It is closing the P11/P12 correctness gate, proving end-to-end resource limits and isolation, reconciling documentation, resolving licensing, and validating customer economics with measured data.


## 14. Additional source-level inspection (first pass)

This section was added after inspecting selected live C, Rust and Nuxt source files on `main`. It is still a targeted review, not a complete line-by-line audit.

### Confirmed implementation facts

- The Rust API's configured API keys are compared using a dedicated constant-time byte comparison helper. The current mechanism is a static server-side key list, not tenant identity, per-user sessions or a complete key lifecycle.
- The Axum API applies a 1 MiB request limit, a queue capacity of one waiting request, and a provisional 10-second deadline. The multipart file is collected into memory; the layout path then owns a byte vector.
- The FIT layout route calls the bounded native output method with a server-owned budget. The public C ABI also continues to expose an unbounded `imgengine_layout_render_v2` symbol. Untrusted/native external callers must use the bounded symbol; the unbounded entrypoint should be documented as trusted-only or deprecated under a compatible policy.
- The C public layout ABI validates canvas edge/area, placement count and reserved fields, copies the bounded placement array, serializes use of the opaque engine lock, and checks that expected/planned/executed counts agree before transferring output ownership.
- The private C renderer checks nonzero rectangles, overflow-safe right/bottom edges, canvas containment and pairwise overlap before rendering.
- The Rust supervisor creates private `0700` request workspaces and removes them on normal completion and Drop. Stale cleanup is performed at startup. This is useful process-lifetime cleanup, not proof of cleanup after SIGKILL, power loss or host failure.
- The Nuxt proxy limits incoming request bytes, rejects unknown/duplicate multipart fields and keeps the API key in private runtime configuration. It buffers the upstream JPEG with `response.arrayBuffer()`; the Rust API also returns a complete in-memory response body. This is bounded by the current FIT output policy, but it is not streaming.
- The C engine startup allocates a fixed 512 MiB slab region via the NUMA allocator (or aligned allocation fallback). This is a reservation/allocation fact; actual resident memory and cgroup charging must be measured on the target VPS.
- The decoder-facing wrapper `img_api_decode_prepared` delegates to the registered decoder vtable. The standalone `img_security_validate_request` helper has a 16,384 × 16,384 pixel ceiling and a 4 GiB RGBA estimate, but this audit has not yet established its call reachability from every decoder path. Trace and test this before relying on it as a security gate.

### New high-priority findings

| ID | Priority | Finding | Required action |
|---|---|---|---|
| SEC-13 | P0/P1 integration blocker | `IMGENGINE_SANDBOX` defaults OFF. When enabled, seccomp uses `SECCOMP_FILTER_FLAG_TSYNC`, which synchronizes the filter across the process thread group. The allowlist does not include the socket/bind/listen/accept/epoll syscall family required by the Axum server. The API runtime starts the native engine before binding its listener. | Do not enable this build flag in the in-process Axum server without a dedicated integration test. Decide whether the sandbox is a whole-process policy or move native decoding into a separate restricted child process. Add an end-to-end test that starts the API with the intended sandbox configuration and serves a real request. |
| SEC-14 | P1 capacity/security | Native startup allocates a fixed 512 MiB slab region. On a small VPS this may consume a material fraction of the container/host memory budget, even before request-specific decoded images and output buffers. | Measure RSS, virtual size, cgroup memory.current/peak, NUMA allocation behavior and OOM headroom on the actual VPS. Make pool sizing a validated server/native configuration if measurements justify it; do not blindly shrink it without allocator tests. |
| SEC-15 | P1 decoder boundary | A 16K × 16K image represents 268,435,456 pixels; an RGBA estimate is 1 GiB, while the validator's stated hard ceiling is 4 GiB. Its invocation path has not yet been proven for all decoders. | Establish a much tighter product-specific pixel/RSS envelope, trace the validator into every decoder entry, and add decompression-bomb tests with the actual TurboJPEG/PNG paths and the 1 MiB API limit. |
| SEC-16 | P1 ABI safety | The public ABI retains an unbounded layout render symbol alongside the bounded variant. | Make bounded processing the only supported route for untrusted inputs. Explicitly mark/document unbounded use as trusted-only and add external consumer tests that prove the HTTP path cannot call it accidentally. |
| SEC-17 | P2 memory/latency | Both Rust API and Nuxt proxy materialize complete request/response bodies in memory; there is no end-to-end streaming path. | Keep strict body/output/concurrency ceilings for MVP. Consider streaming only with backpressure, disconnect, error and cleanup semantics designed and measured; do not add streaming as a superficial refactor. |

### Sandbox reasoning and limitation

The sandbox lifecycle verifier configures a separate CMake build with `IMGENGINE_SANDBOX=ON` and runs a dedicated C sandbox lifecycle test. The normal build defaults the option to OFF. That test is not equivalent to proving that an Axum HTTP process can start, bind, accept connections and shut down under the same TSYNC filter. This integration must be resolved before describing the Rust API deployment as sandboxed.

### Immediate source-review order

1. Reproduce the sandbox/API interaction in an isolated Ubuntu 24.04 Docker test; do not change the production default during diagnosis.
2. Trace every decoder implementation and prove width/height/pixel validation occurs before large allocations.
3. Measure the 512 MiB slab's actual memory behavior under the target cgroup limits.
4. Inspect the exact bounded JPEG allocator path and document the unbounded ABI method's supported trust model.
5. Add an end-to-end memory budget covering Nuxt, Axum, native pool, decoded source, canvas, encoder output, response copies and concurrent requests.
