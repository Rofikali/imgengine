# IMGENGINE frontend and legacy backend documentation audit — 2026-11-14

**Baseline:** `fd176d33852a5e6f2e615db455870e89172969da` (`main` at audit start). **Scope:** `imgengine-saas/frontend/`, `imgengine-saas/backend/`, their two scoped `AGENTS.md` files, relevant product/architecture/security/operations documents, proxy/worker source and CI descriptions. **No source, dependencies, runtime configuration, CI, build scripts, or deployment changed by this audit.** No tests, vulnerability scans or performance benchmarks were run for this documentation change. Old recorded test results are explicitly dated.

This complements the [81-file C/Rust documentation inventory](NATIVE_C_RUST_DOCS_MAP.md) and [native audit](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md), rather than duplicating or superseding them.

## Critical instruction-scope distinction

- [Root AGENTS.md](../AGENTS.md) applies repository-wide.
- [Frontend AGENTS.md](../imgengine-saas/frontend/AGENTS.md) governs the active **Nuxt/Vue/TypeScript** UI and server-side proxies. It does not govern the C scheduler or Rust FFI.
- [Backend AGENTS.md](../imgengine-saas/backend/AGENTS.md) governs files under `imgengine-saas/backend/`. **That directory contains retained Python/FastAPI/Celery/worker code.** Its Rust target architecture is a migration design, not evidence that Rust sources live here.
- Active Rust/Axum, safe FFI and supervisor source is under `imgengine/rust/`, governed by [native AGENTS.md](../imgengine/AGENTS.md) plus root policy. Never infer Rust code is governed by the legacy backend AGENTS simply because it describes Rust.
- [skills.md](../skills.md) is an **optional** workflow playbook, not an automatically installed/loaded skill; [next.md](../next.md) tracks exact status and open questions; [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md) governs **local test choice**, not CI suppression.

## Status classification

| State | Meaning |
| --- | --- |
| IMPLEMENTED | Source path and contracts currently exist; no fresh test claim |
| VERIFIED (RECORDED) | Dated exact-scope test evidence was recorded on an earlier revision/environment |
| PARTIAL | Some layers exist, but full workflow, security, or release evidence is incomplete |
| PLANNED | Future design, not yet a supported product feature |
| LEGACY | Older asynchronous/durable FastAPI/worker architecture kept for controlled migration |
| HISTORICAL | Old document preserves dated factual context; may no longer describe `main` |
| OPEN / REQUIRES EVIDENCE | Specific question or release gate not yet proven; not necessarily a vulnerability |

## Frontend: source-aligned implementation matrix

| Feature / responsibility | Status | Source or recorded evidence | Non-claim / next proof |
| --- | --- | --- | --- |
| Nuxt 4.6.4 / Vue 3.5.44, TS, Corepack pnpm 10.34.9 | IMPLEMENTED; prior build verified | `frontend/package.json`, `pnpm-lock.yaml`, [P14 baseline](P14_PRODUCTION_BASELINE.md) | Frontend changes still require fresh tests at new SHA |
| Browser-to-same-origin API client | IMPLEMENTED | `frontend/composables/useImgengineApi.ts`, `frontend/types/api.ts` | Not a user-account/tenant auth layer |
| Synchronous P5 JPEG render | IMPLEMENTED | `frontend/server/api/render.post.ts`, `composables/useRenderWorkflow.ts` | Legacy `/api/jobs` not part of active Rust path |
| P8 FIT layout proxy | IMPLEMENTED | `frontend/server/api/layout-render.post.ts`, `composables/useLayoutRenderWorkflow.ts` | Do not silently change FIT semantics |
| P12 physical centered-fill JPEG sheet | IMPLEMENTED; local evidence | `frontend/server/api/sheet-render.post.ts`, `components/SheetRenderPanel.vue`, `useSheetRenderWorkflow.ts` | Native-confirmed counts, canvas; no passport-compliance guarantee |
| Private Rust credential boundary | IMPLEMENTED CONTROL | `frontend/nuxt.config.ts` private `runtimeConfig`; server proxies add `X-API-Key` | Audit secret provenance/config at exact release; browser cannot authorize itself |
| Ingress size check | IMPLEMENTED CONTROL | Three server proxy modules use 1,048,576-byte limit and bounded stream before multipart parsing | Rust remains authoritative; no proof of total process/response RSS cap |
| Safe error mapping, abort and preview cleanup | IMPLEMENTED | `utils/api-errors.ts`, `useSheetRenderWorkflow.ts` abort controller / URL.revokeObjectURL | Browser abort **does not cancel C processing** after admission |
| Frontend unit/integration-style tests | VERIFIED (RECORDED) | `frontend/**/*.test.ts`; Nov 2026 P14 records 36 Vitest tests, typecheck and build | No fresh run in this docs PR; browser automation / accessibility evidence separate |
| P13 user-controlled crop UI/transport | PARTIAL | C ABI 2.3 exists; current Nuxt P12 proxy does not accept crop coordinates | M1.1 Rust FFI/API/proxy then M1.2 accessible UI and M1.3 parity |
| Local browser Worker/Rust-Wasm/compression/tools | PLANNED | [Product roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) | No first-party browser-local engine proven |
| PDF, PNG sheet export, batch, subscriptions, user accounts | PLANNED | P12 explicitly JPEG only; no these routes in inspected frontend | Do not advertise shipping capabilities |
| Real-VPS, public canary and SLA | OPEN | [MVP checklist](MVP_RELEASE_CHECKLIST.md), [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) | Docker Desktop != actual host |

### Frontend review findings (documentation, not source fixes)

| ID | Priority | Finding | Verification / future action |
| --- | --- | --- | --- |
| FE-01 | P0 | Three Nuxt server proxies repeat bounded-body/error forwarding code. This creates maintenance drift risk; no duplicated behavior regression is proven. | Review a future shared tested adapter only after contract-specific field and error differences are frozen; no refactor in docs PR |
| FE-02 | P0 | Browser-side cancellation only aborts the client request and revokes object URLs. Rust/C native work is not mid-call cancellable. | Keep response-abandonment language; assess memory/deadline semantics |
| FE-03 | P0 | The single private Rust render API key is a server-to-server credential, **not an end-user identity, tenant isolation or paid quota**. | Future auth/abuse threat model before opening to general public traffic |
| FE-04 | P1 | Proxy streams are bounded on ingress, but end-to-end memory (multipart buffering, arrayBuffer response and concurrent results) is not proven by the 1 MiB request cap. | Profile full process / cgroup under P14 exact-host matrix |
| FE-05 | P1 | Historical moderate/low Nuxt/Nitro production advisories remained after Nov 13 fix; CI reports lower severity and fails on critical/high. | Fresh lockfile audit at release; owner accepts or remediates outstanding risks |
| FE-06 | P1 | Accessibility guidelines exist; semantic labels and UI state messages are present. No repository-wide accessible browser E2E gate was established by this audit. | Manual and automated keyboard/mobile/screen-reader checks for P13 and P12 release |
| FE-07 | P1 | `nuxt.config.ts` sets `devtools: { enabled: true }`. This does **not alone prove production exposure**. | Verify production-build behavior before release and configure intentionally |
| FE-08 | P1 | Current app has one sheet component and specific workflows, not a general image-editor route inventory. | Product, accessibility and SEO decisions separately; avoid claims of 100% tool coverage |
| FE-09 | P2 | Old P7.0 audit correctly recorded a Nuxt 3 / legacy job UI at its historic revision. | Keep as historical, link implemented Nuxt 4 + P7.2/P12 outcome rather than re-opening old findings |

## Legacy backend: source-aligned implementation matrix

The Python source is **retained, not selected as the target first-MVP service**. This distinction is essential for security, cost and engineering decisions.

| Feature / responsibility | Status | Existing source | Non-claim |
| --- | --- | --- | --- |
| FastAPI HTTP app, `/api/generate` and internal job update | LEGACY / IMPLEMENTED SOURCE | `backend/api-service/app/main.py`, `app/api/routes/generate.py`, `internal.py` | Does not make those routes part of the Rust/Axum API |
| Celery worker, Redis broker, PostgreSQL job/events, SQL migrations | LEGACY / IMPLEMENTED SOURCE | `backend/worker-service/tasks.py`, `app/core/celery_client.py`, `app/core/db.py`, migrations | Not current synchronous ephemeral deployment dependency |
| Private artifact store, output status and retention | LEGACY / IMPLEMENTED SOURCE | `app/core/storage.py`, `job_states.py`, legacy SaaS contract | Not permission to persist images on Rust path |
| SHA-256 API key ownership fingerprint and token verification | LEGACY / IMPLEMENTED SOURCE | `app/core/security.py` | Not a full tenant identity and API key lifecycle system |
| JPEG/PNG signature check and bounded legacy upload | LEGACY / IMPLEMENTED SOURCE | `app/core/upload_validation.py`, `generate.py` | Signature alone does not prove full valid image decode/security |
| Production configuration validation | LEGACY / IMPLEMENTED SOURCE | `app/core/config.py` rejects known development secrets when DEPLOYMENT_ENV=production | Must verify actual deployment environment and secret mount |
| Legacy logs/Prometheus/tracing | LEGACY / IMPLEMENTED SOURCE | `main.py`, `app/core/metrics.py`, `tracing.py`, Celery task metrics | Not evidence of Rust/Axum production telemetry |
| Legacy database backup/restore scripts | LEGACY / SOURCE+DATED EVIDENCE | [DATABASE_RECOVERY](DATABASE_RECOVERY.md) and `imgengine-saas/scripts/` | Not Rust MVP requirement |
| Rust/Axum current backend | IMPLEMENTED elsewhere | `imgengine/rust/imgengine-api`, `imgengine-supervisor`, `imgengine` | **Not contained in** `imgengine-saas/backend/` |
| Full retirement of Python/Celery/Redis/Postgres | OPEN / DEFERRED | [migration](migration.md), [legacy release checklist](RELEASE_CHECKLIST.md) | Requires parity/canary/rollback and explicit decision |

### Legacy backend security/operations review hypotheses

| ID | Priority | Source observation | Next evidence, not an exploit claim |
| --- | --- | --- | --- |
| BE-01 | P0 | `verify_internal_token` uses normal string inequality while API-key matching uses `hmac.compare_digest`. | Confirm route exposure and threat context; consider constant-time secret comparison in a scoped security PR |
| BE-02 | P1 | `main.py` accepts a client-supplied `X-Request-ID` as correlation ID on the legacy path. | Validate format/length/control characters or generate trusted ID; redaction tests |
| BE-03 | P1 | Legacy `/metrics` is registered without authentication in `main.py`. | Verify it is private at actual network layer and whether it exposes sensitive metadata |
| BE-04 | P1 | Local development default API/internal tokens exist in `config.py`, with a production mode validator. | Prove production mode and invalid-config startup fail-closed behavior on any legacy deployment |
| BE-05 | P1 | Legacy stdout/stderr and job event persistence need source-traced redaction and retention checks. | Validate log sanitization and authorized access; do not claim current Rust logging inherits these guarantees |
| BE-06 | P1 | Durable-job cleanup, broker failure and idempotency require their own operational recovery tests. | Preserve [legacy SRE](SRE.md), [backup](DATABASE_RECOVERY.md), [SaaS contract](SAAS_CONTRACT.md) if legacy service is exposed |
| BE-07 | P2 | Large historical commented worker prototype remains in `tasks.py`; operational docs reference it only as historical. | No code cleanup under this PR; cleanup later if migration/retirement authorizes |

## Related documentation classification and authority

| File | Current role |
| --- | --- |
| [P7_FRONTEND_ARCHITECTURE_AUDIT.md](P7_FRONTEND_ARCHITECTURE_AUDIT.md) | Historical P7.0 Nuxt 3 / durable-job mismatch, later corrected for active path |
| [P7_FRONTEND_DEV_ENVIRONMENT.md](P7_FRONTEND_DEV_ENVIRONMENT.md) | Recorded Ubuntu/Docker/Node/pnpm environment; some older “Nuxt upgrade deferred” language is historical |
| [P7_RECOVERY_INTEGRATION_REVIEW.md](P7_RECOVERY_INTEGRATION_REVIEW.md) | Historical local Windows worktree rescue; do not infer current local worktree cleanliness |
| [api.md](api.md) | Current Rust P5/P8/P12 API and resource/error contract |
| [P12_PHYSICAL_SHEET_CONTRACT.md](P12_PHYSICAL_SHEET_CONTRACT.md) | Implemented physical JPEG sheet; release open |
| [P13_INTERACTIVE_CROP_CONTRACT.md](P13_INTERACTIVE_CROP_CONTRACT.md) | Native ABI 2.3 implemented; Rust/front UI incomplete |
| [OBSERVABILITY.md](OBSERVABILITY.md) | Legacy Python/Celery log, DB audit and trace context; not current Rust observability specification |
| [SECURITY_OPERATIONS.md](SECURITY_OPERATIONS.md) | Legacy durable-job owner/idempotency/rate controls; not current Rust paid tenancy |
| [SRE.md](SRE.md) | Legacy queue/job success SLOs, not public Rust SLO |
| [DATABASE_RECOVERY.md](DATABASE_RECOVERY.md) | Legacy PostgreSQL backup/restore, not first Rust MVP |
| [SAAS_CONTRACT.md](SAAS_CONTRACT.md) | Legacy API/worker/storage contract |
| [PRD.md](PRD.md) | Earlier asynchronous product-generation PRD; not current Rust/P12 authority |
| [API_CONTRACT_PARITY_GATE.md](API_CONTRACT_PARITY_GATE.md) | Historical P5 parity evaluation; later additive endpoints supersede “one capability only” descriptions |
| [P14_PRODUCTION_BASELINE.md](P14_PRODUCTION_BASELINE.md) | Local hardening/dependency posture and open release evidence |
| [MVP_RELEASE_CHECKLIST.md](MVP_RELEASE_CHECKLIST.md) | Current first Rust physical-sheet MVP release go/no-go |
| [PRODUCT_ROADMAP_BROWSER_FIRST.md](PRODUCT_ROADMAP_BROWSER_FIRST.md) | Future tools/Wasm/paid plans; not shipped capability |
| [CHANGE_SCOPED_VERIFICATION.md](CHANGE_SCOPED_VERIFICATION.md) | Change-scoped local testing; no automatic CI path filters |
| [NATIVE_C_RUST_DOCS_MAP.md](NATIVE_C_RUST_DOCS_MAP.md) | Broader historical/current documentation authority |

## Architecture / SOLID / DSA / design-pattern scope

- **Frontend:** component-composable-service adapter layering supports separation of concerns; `useImgengineApi` is a Facade/Adapter over three versioned routes. The repeated server-proxy stream validation is a potential DRY maintenance issue. No formal complete SOLID certification is made.
- **Legacy backend:** FastAPI route + service/DB/storage/Celery boundaries demonstrate multiple responsibilities, not a certified DDD/Hexagonal implementation. Durable job states, idempotency and retries require actual state-transition invariants and tests; adding design patterns is not itself a migration goal.
- **Active Rust/C:** target control/execution separation is covered by the canonical [native matrix](NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md). Do not duplicate the C image engine in Nuxt or legacy Python.

## Business / CA-MBA decision rule

Avoid paying the maintenance and security cost of **two** public backends without a demand/rollback justification. Preserve legacy only as approved migration insurance, and meter operational overhead separately (backup, worker, DB, broker) from Rust/C ephemeral operations. Proposed paid routes, student presets and advanced studio features must show validated customer demand and cost-per-successful-job; none should be described as existing revenue.

## Verification policy and follow-up

Documentation-only: review links, file scope, status claims and diff. No manual CMake, ASan, UBSan, native fuzz, Rust integration or Nuxt rebuild for this PR; **required CI continues normally**. Subsequent frontend-only PRs: targeted Vitest/typecheck/build/browser UX/accessibility. Legacy backend changes: scoped Python route, storage, auth and recovery tests; Rust/Axum/FFI changes: Rust/ABI/security/integration gates. All new security claims require exact-head test evidence.

No production readiness, tenanted security, legal compliance, universal 30 ms image processing, browser-local image privacy or customer profitability claim follows from this audit.
