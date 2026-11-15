# IMGENGINE — Project inventory and decision register

**Initial snapshot:** 2026-11-14, `main` at `14d1a8683f492ec2c1128a2014442cc85cd832e9`. **Layout/status reconciliation:** `main` at `e567616975476204c9279f476c28fb74e67512bc`, after PR #37 merge (2026-11-14). This is a living index, not a release certificate. Status is based on repository contracts, tracked code/PRs, and recorded verification, not fresh end-to-end execution.

## Status vocabulary
- **Implemented:** tracked code exists; may not have complete evidence.
- **Validated (recorded):** tests or review evidence exists for a specified revision/environment; not necessarily reproduced today.
- **Partial:** a lower layer exists, but the customer-visible workflow is incomplete.
- **Planned:** proposal only, no implementation claim.
- **Blocked:** prerequisite or operational evidence missing.
- **Legacy:** retained during controlled migration; do not delete without parity and rollback.
- **Historical:** superseded design/evidence; retain for traceability.

## Built and/or validated so far

| Area | Status | Source / evidence | Remaining work |
| --- | --- | --- | --- |
| C native engine, scalar/SIMD, TurboJPEG, scheduler | Implemented; recorded native tests | `imgengine/`, `docs/ENGINE_SPEC.md`, `docs/SCHEDULER_CHARACTERIZATION_REPORT.md` | Preserve correctness, measure workload-specific performance |
| C17 shipping baseline and C11 compatibility | Validated (recorded) in Ubuntu 24.04 Docker | PR #29, `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md` | Keep C11 regression and ABI gates on native changes |
| Public C ABI v1 and additive v2.0–2.3 | Implemented; recorded consumer/export/layout checks | `docs/ABI.md`, `docs/P13_INTERACTIVE_CROP_CONTRACT.md` | Maintain compatibility on future native changes |
| Rust FFI, supervisor, bounded admission, Axum | Implemented; recorded integration tests | `docs/RUST_FFI.md`, `docs/RUST_SUPERVISOR.md`, `docs/api.md` | Production operational evidence |
| P8 FIT layout | Implemented | `docs/P8_PRODUCT_CONTRACT.md` | Keep unchanged while adding new tools |
| P11 geometry planning | Implemented | PR #13, `docs/P12_PHYSICAL_SHEET_CONTRACT.md` | Preserve deterministic parity |
| P12 physical JPEG sheet and Nuxt UI | Implemented; local workflow demonstrated, not production | `docs/P12_PHYSICAL_SHEET_CONTRACT.md`, PRs #16–19, #18 | VPS/resource evidence, usability release gates |
| P13 normalized native crop, ABI 2.3 | **Partial** | PRs #24–26, `docs/P13_INTERACTIVE_CROP_CONTRACT.md` | Rust/Axum/Nuxt transport, accessible UI, end-to-end parity |
| Nuxt 4 frontend and typed proxy | Implemented | `imgengine-saas/frontend/`, frontend `AGENTS.md` | Crop UI, browser-local engine, broader tools |
| Docker / Caddy private candidate | Implemented local candidate; production blocked | `docs/P12_DEPLOYMENT.md`, `docs/LOCAL_DOCKER_REHEARSAL.md` | Real VPS, TLS/DNS/firewall, rollback, monitoring |
| Native sanitizer/fuzz/ABI verification | Validated (recorded), not run for this documentation change | `imgengine/scripts/verify_linux.sh`, PR #29 | Re-run on relevant native changes and release gates |
| P7.5 worktree changes | **Separate local dirty worktree reported by operator** | `docs/P7_5_BRANCH_CONSOLIDATION.md` records older integrated P7.5 history | Reconcile local uncommitted work separately; remote GitHub cannot inventory local dirt |
| Python/FastAPI/Celery/Redis/Postgres path | **Legacy** | `AGENTS.md`, `docs/migration.md` | Retire only after parity, rollback window and explicit decision |
| Browser-first Rust/Wasm processing | **Planned, not implemented** | M0 ADR | Architecture review, bounded proof, browser tests |
| Tailwind | **Not established as implemented** in the 2026-11-14 audit | Audit snapshot / frontend files | Choose deliberately; avoid asserting installed |
| Paid tenancy, quotas, metering, B2B | **Planned** | `docs/PRD.md`, roadmap | Demand validation, privacy and payment architecture |

## Unfinished, interrupted or deferred work
1. **P13/M1 interactive crop:** native ABI complete, transport/UI incomplete. Next implementation boundary.
2. **P12/P14 public release:** local candidate exists; actual-VPS RAM/CPU/disk, TLS, monitoring, canary and rollback evidence missing. Do not claim production-ready.
3. **P7.5 separate worktree:** preserve as-is; inspect locally before any reconciliation. A remote branch or PR does not prove its uncommitted state.
4. **Browser-local processing:** no first-party Worker/Wasm engine. Transitive Wasm dependencies are not product functionality.
5. **Image product catalog:** crop, resize, compression, conversion, signature tools, presets, batch, professional editor, photo sheets in browser are a proposed expansion, not shipped features.
6. **Legacy retirement:** explicitly deferred until replacement parity and rollback evidence.
7. **Nuxt dependency advisories:** verify current advisory state and record accountable disposition; historical audit mentioned moderate/low findings.
8. **Branch protection:** at last repository inspection `main` was unprotected; establish required checks/reviews according to repository capabilities.

## Approved technology and directory decision (2026-11-14)

**ADR-TECH-001 accepted for all future product work:** C17 native core; safe Rust FFI; Rust/Axum/Tokio HTTP/backend; existing Nuxt/Vue/TypeScript frontend. No new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, or Python scripts/tooling. This is a **new-work policy**, not a claim that the retained legacy FastAPI/Celery/Redis/PostgreSQL sources or old Python verification scripts have been removed. Active Rust source is now top-level `backend/` after merged PR #37; `imgengine-saas/backend/` remains legacy Python only. **Legacy** folder moves, code deletion and retirement remain **DEFERRED** until independently authorized with dependency paths, tests, data disposition and rollback. See [technology and structure ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md) and [Rust AGENTS](../backend/AGENTS.md). Future paid metadata storage is undecided and requires separate explicit approval; avoid introducing PostgreSQL by default.

## Decision log
| ID | Decision | Reason / revisit |
| --- | --- | --- |
| D-001 | Keep C scheduler and native processing plane | Existing investment and characterization; revisit only on measured evidence |
| D-002 | Shipping C17, C11 regression, isolated C23 experiments | PR #29; preserve ABI |
| D-003 | Browser-first local processing is a product goal, not a shipped capability | Minimize uploads and server compute; prove actual browser resource envelope |
| D-004 | Start Wasm architecture review after crop M1, before production Wasm code | Avoid parallel architecture rewrite and premature codec port |
| D-005 | Prefer browser-native codecs plus bounded Worker geometry prototype first | M0 ADR; compare parity, licensing, memory and bundle size |
| D-006 | No automatic server fallback for sensitive images | Require explicit consent and clear privacy disclosure |
| D-007 | Change-scoped verification locally; full relevant gates on integration/release | Save developer time/tokens without sacrificing release evidence |
| D-008 | No blanket promise of 30 ms, 60 FPS, 1 GiB VPS capacity, passport compliance, or guaranteed target-KB quality | Require workload-specific evidence |
| D-009 | No production or public release without explicit human go/no-go | Security, legal, cost and operational risk |

## Nuxt frontend and legacy backend reconciliation (2026-11-14)

See [frontend/legacy backend source audit](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) for per-feature source references, agent scope and open questions. **Implemented / prior recorded evidence:** Nuxt 4.6.4, typed P5/P8/P12 same-origin proxies, private server-only Rust credential, bounded multipart ingress, client preview cleanup, unit tests/build recorded Nov 2026; retained Python/FastAPI/Celery/Redis/PostgreSQL job backend source and historical backup/SRE contracts. **Partial:** interactive P13 crop transport/UI, accessibility/browser E2E evidence and overall memory budget. **Planned:** local Worker/Wasm image processing, PNG/PDF sheet output, batch, paid users/tenants. **Open:** historic production dependency advisories, browser production hardening, actual VPS capacity/TLS/canary, and legacy service security hypotheses (internal-token comparison, request-ID trust, metric endpoint reachability). Legacy Python source is not the location of the Rust backend, which now lives in top-level `backend/`.

## Astra engineering handoff and structural verification

- **DONE / MERGED:** Rust backend crates moved to top-level `backend/` in [PR #37](https://github.com/Rofikali/imgengine/pull/37), 7/7 CI jobs passed at exact PR head `4ab5a65fcde411fd77dde8eb36aea09dce7915c3` ([run](https://github.com/Rofikali/imgengine/actions/runs/43618598771)). This does not close P13 or P14.
- **PARTIAL:** P13 native ABI 2.3 is implemented; Rust adapter, HTTP/proxy, accessible Nuxt UI and parity evidence remain separate milestones.
- **OPEN / review-only first:** C observability concurrency, aggregate memory envelope, deployment isolation and current production release proof.
- **Source of handoff:** [Astra start guide](ASTRA_ENGINEERING_START_HERE.md) defines three bounded frontend/backend/native tracks, quality gates and CA/MBA business checks. Do not convert its proposals into shipped features.

## Active infrastructure security and economics audit (2026-11-15)

The [18-file active infra audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) and [scoped infra AGENTS](../infra/AGENTS.md) distinguish **P7 developer tools**, **P8 Docker Desktop VPS simulation**, **P12 disposable candidate**, **P14 hardened production Compose configuration**, and **unverified real-host deployment**. **Implemented/configured:** Caddy-only public entry, internal Rust network, non-root/read-only services, image-digest variables, file-backed secrets, basic healthchecks and cgroup ceilings; some local P12/static CI evidence was recorded previously. **OPEN/P0 for release:** host-file secret metadata/access (Compose `file:` ignores `uid/gid/mode` fields), abuse/request economics and actual host memory/CPU/tmpfs under hostile workloads. **OPEN/P1:** P8 image-base/offline-cache reproducibility, misleading dev-toolchain inheritance, static verifier privilege blind spots, runtime process-env secret handoff, Caddy/TLS/alerts, SBOM/digests, rollback. None are claimed as deployed exploits. The active infrastructure review is an **independent Astra Track D**, not permission to deploy or introduce a new database/queue/Python verifier.

## Keeping this register current
For each PR: update relevant status, source links, owner/decision, blockers, next action, and exact revision tested. Do not mark an item validated from an AI summary alone. Historical records remain immutable; corrections should be dated. Cross-check `docs/PRODUCT_ROADMAP_BROWSER_FIRST.md` and `docs/CHANGE_SCOPED_VERIFICATION.md`.
