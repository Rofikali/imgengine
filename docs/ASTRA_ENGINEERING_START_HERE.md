# Astra start here — IMGENGINE frontend, Rust backend and C17 core

**Document purpose:** bounded engineering-agent handoff and audited start sequence. **Repository baseline:** `main` merge commit `e567616975476204c9279f476c28fb74e67512bc` (PR #37 merged, 2026-11-14). **This is not a new code/test result or a release certificate.** Always inspect current Git SHA, diff, exact implementation and required contracts before making changes. If this baseline is stale, update the evidence and status before relying on it.

**Scope:** Nuxt/Vue/TypeScript frontend; Rust/Axum/Tokio backend, request supervisor and safe C FFI; portable C17 native SDK and ABI; security, privacy, observability, HLD/LLD, DSA, SOLID and performance; delivery and product economics. **Not authorized:** new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, database/queue/tooling replacements, unrelated infrastructure, public deployment or a new paid feature without explicit separate decision. Existing Python code and safety scripts remain retained legacy/tooling until reviewed retirement/replacement.

## 1. Authority and document navigation

**Do not paste the whole documentation tree into one agent context** (91 Markdown files in `docs/` on this infra-audit branch). Read this concise index, root [AGENTS](../AGENTS.md), [next.md](../next.md), [skills](../skills.md), and [approved stack](TECHNOLOGY_AND_REPOSITORY_POLICY.md). Then read only the affected scoped AGENTS, source, versioned API/ABI and design/evidence documents. The source and exact-revision tests outrank old plans and aspirational HLD. A root `skills.md` is a playbook, **not automatically installed as a model skill**.

| Track | Mandatory scoped files | Relevant canonical technical contracts |
| --- | --- | --- |
| **Frontend** `imgengine-saas/frontend/` | [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md); [frontend/legacy audit](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) | [P12 current sheet](P12_PHYSICAL_SHEET_CONTRACT.md), [P13 interactive crop](P13_INTERACTIVE_CROP_CONTRACT.md), [Rust HTTP contract](api.md), [product roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) |
| **Rust backend** `backend/` | [backend AGENTS](../backend/AGENTS.md); [backend README](../backend/README.md) | [Rust FFI](RUST_FFI.md), [lifecycle](RUST_REQUEST_LIFECYCLE.md), [supervisor](RUST_SUPERVISOR.md), [HTTP contract](api.md), [native/Rust contract matrix](NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) |
| **C17 native** `imgengine/` | [native AGENTS](../imgengine/AGENTS.md); [native development](NATIVE_DEVELOPMENT.md) | [ABI](ABI.md), [exports](abi/exported_symbols.md), [native source/evidence audit](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md), [engine spec](ENGINE_SPEC.md), [native benchmark contract](NATIVE_BENCHMARKING.md) |
| **Infrastructure** `infra/` (independent review-only Track D) | [infra AGENTS](../infra/AGENTS.md); [infra README](../infra/README.md) | [infra source audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [P14 baseline](P14_PRODUCTION_BASELINE.md), [actual VPS runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| **Cross-cutting security/release/business** | [MVP release checklist](MVP_RELEASE_CHECKLIST.md), [actual VPS runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) | [threat model](threat-model.md), [security design](security.md), [scoped testing](CHANGE_SCOPED_VERIFICATION.md), [project decisions](PROJECT_STATUS_AND_DECISION_REGISTER.md), [roadmap and economic hypotheses](PRODUCT_ROADMAP_BROWSER_FIRST.md) |

The older `imgengine-saas/backend/` and its [AGENTS](../imgengine-saas/backend/AGENTS.md) govern **retained Python legacy**, not Rust. Legacy [PRD](PRD.md), [SRE](SRE.md), [database recovery](DATABASE_RECOVERY.md) and [observability](OBSERVABILITY.md) are historical/legacy-scoped; do not use their FastAPI/worker/SQL instructions as the target architecture.

### Track D — Infrastructure (additional independent review-only workstream)

Astra can review **active `infra/`** without waiting for Rust P13 work, but should not change live deployments or unrelated code. Read [infra AGENTS](../infra/AGENTS.md), [infra README](../infra/README.md), [dated infrastructure audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), and actual `infra/docker-compose.production.yml`, `infra/Caddyfile.production` and `infra/scripts/verify-production-compose.py`. Review **INFRA-01** file-backed secret permission semantics and **INFRA-06** static-verifier blind spots first; then app abuse/rate control, aggregate VPS memory, P8 reproducibility, TLS/ACME, alerts, rollback and cost per successful job. Deliver a **source-backed HLD/LLD threat/evidence matrix and focused non-deploy fix plan**. Any changes to config/CI/services need their own approved implementation PR and required tests. No real-host action without explicit go/no-go. The separate `imgengine-saas/infra/` remains legacy.

## 2. Source-backed readiness at the audited baseline

| Capability | Status | Verified/observed scope | Missing before claim of completion |
| --- | --- | --- | --- |
| C17 core, C11 compatibility, current C ABI and SIMD/scheduler narrow cases | **DONE / RECORDED** | Prior M0, P7.4 and P7.5 exact-scope evidence; public ABI 1.0–2.3 | Does not prove all security/performance claims on every platform |
| Native `.so` and Rust FFI | **IMPLEMENTED** | Shared C library, safe Rust facade, resource-release invariants | Public installable `.a` SDK and cross-platform distribution remain open |
| Rust `backend/` relocation | **DONE / MERGED**, verified | PR [#37](https://github.com/Rofikali/imgengine/pull/37), exact head `4ab5a65fcde411fd77dde8eb36aea09dce7915c3`, [7/7 CI run](https://github.com/Rofikali/imgengine/actions/runs/43618598771); merged as `e567616975476204c9279f476c28fb74e67512bc` | This proves the moved layout at that revision, **not** public deployment |
| Rust P5/P8/P12 endpoints/admission | **IMPLEMENTED / RECORDED** | Axum adapter, supervisor, safe FFI, typed existing operations | User-controlled ABI 2.3 crop transport and native mid-operation cancellation not implemented |
| Nuxt 4/Vue/TS proxy/UI | **IMPLEMENTED / RECORDED** | Typed P5/P8/P12 routes, abort and object URL cleanup | P13 accessible manual crop UI, browser E2E and full production accessibility evidence |
| P13 crop | **HALF IMPLEMENTED** | Native normalized crop geometry + ABI 2.3 done | Safe Rust adapter → Axum request → Nuxt same-origin proxy → accessible drag/zoom UI → parity evidence |
| Native and API security | **CONTROLS EXIST; EVIDENCE PARTIAL** | Input validation, output caps, admission, private proxy, sanitizer/fuzz records | Aggregate decoded+scratch+output RSS under concurrency; actual decoder sandbox reachability; potential native metrics synchronization issue |
| Observability and HLD/LLD/design principles | **DOCUMENTED / PARTIAL PROOF** | Existing C telemetry modules, Rust lifecycle, old design-pattern examples and source-aligned matrix | Actual current operational metrics, synchronization, redaction, trace boundaries, source-specific invariants; do not call drawings certified architecture |
| Public release on low-cost VPS | **BLOCKED** | Local Docker/candidate and P14 runbook | Host-specific TLS, secrets, RSS, canary, recovery/rollback, operations, go/no-go |
| Legacy Python retirement | **DEFERRED** | Old backend still tracked, now separate from active Rust source | Proven parity, preservation/disposition of legacy data, rollback and explicit approval |
| Business SaaS, accounts, billing, tenancy | **PLANNED / UNDECIDED** | Strategy, unit-economics questions and optional product routes | Customer evidence, operating cost, approved durable metadata choice and threat model |

**Security findings are review hypotheses**, not reproduced vulnerabilities. **Performance goals are target-specific**, never universal nanosecond/microsecond image-processing promises.

## 3. Astra launch sequence — one bounded PR/milestone at a time

### Track A — Rust backend first: P13 M1.1 transport contract

**Outcome:** make the *already implemented* native C ABI 2.3 normalized crop safely callable from Rust and the correct Axum/same-origin contract, while keeping P12 ABI 2.2 centered-fill behavior unchanged.

**Start:** [P13](P13_INTERACTIVE_CROP_CONTRACT.md), [P12](P12_PHYSICAL_SHEET_CONTRACT.md), [Rust FFI](RUST_FFI.md), [Rust HTTP](api.md), `backend/imgengine/src/lib.rs`, `backend/imgengine-api/src/lib.rs`, `backend/imgengine-supervisor/src/`, C [sheet ABI](../imgengine/include/imgengine/api/v2/sheet.h), frontend `server/api/sheet-render.post.ts`.

**Astra's first deliverable:** an implementation-ready **contract/interface review** with exact struct fields/version, Rust ownership/lifetime, error classes, validation, resource envelope, cancellation/response-abandonment behavior, test fixtures and compatibility matrix. Then implement only the approved M1.1 adapter and tests in a *separate focused implementation PR*. Do not rewrite C crop geometry or silently alter P8 FIT/P12 centered-fill semantics.

**Pass:** invalid crop geometry rejected at correct boundary; exact-safe error mapping; Rust FFI output freed on all outcomes; typed API/client data compatibility; P12 old requests regressions pass; focused Rust/API/ABI tests and required CI at exact head. No unapproved new format/DB/queue.

### Track B — Nuxt frontend next: P13 M1.2 interactive accessible UI

**Dependency:** Track A contract frozen, tested and approved before implementation. UI architecture/a11y design can be reviewed in parallel without editing the same API fields.

**Start:** [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [frontend audit](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), `imgengine-saas/frontend/components/SheetRenderPanel.vue`, `composables/useSheetRenderWorkflow.ts`, `server/api/sheet-render.post.ts`, [P13](P13_INTERACTIVE_CROP_CONTRACT.md).

**Astra's deliverables:** interaction state contract for keyboard/touch/mouse drag, bounded zoom, reset, deterministic crop coordinates, accessible labels/focus/live errors; safe preview URL cleanup and no secret exposure; P12 default still centered-fill. Then one small UI implementation PR with component and browser/a11y tests. No image-data telemetry and no claim that browser abort cancels C processing.

**Pass:** correct P13 geometry and pixel parity vs native fixture oracles; mobile/touch/keyboard work; files and URL memory freed; invalid inputs and request failure states covered; Nuxt unit/typecheck/build plus relevant browser tests and exact-revision CI. **Do not run manual native ASan/fuzz for a pure UI change** unless shared ABI/FFI code was touched.

### Track C — Native C17 security and SDK hardening: evidence-first review

**Can begin independently of Track A/B as a review-only workstream.** Do not make broad C and Rust/FE changes in one PR.

**Start:** [native AGENTS](../imgengine/AGENTS.md), [native audit](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md), [contract matrix](NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md), `imgengine/src/observability/metrics.c`, `src/security/input_validator.c`, `src/security/sandbox_apply.c`, [native benchmark rules](NATIVE_BENCHMARKING.md), `imgengine/CMakeLists.txt` and public ABI headers.

**Astra's first deliverable:** a source-traced, *no-code* risk register and HLD/LLD deltas addressing (1) possible min/max metric concurrency race and caller reachability; (2) aggregate memory/CPU/workspace envelope beyond JPEG output cap; (3) actual decoder process/seccomp boundary and failure modes; (4) installed shared vs public static SDK and CPU portability; (5) FFI ownership and ABI compatibility; (6) exact benchmark fixture and baseline. Classify every concern as confirmed, unverified or already closed with dated evidence.

**After review:** submit separate tiny PRs **only where evidence confirms defects or approved SDK requirements**. For C/ABI/unsafe FFI changes, preserve C17 shipping, C11 consumer tests, scalar oracle/SIMD parity, symbol/struct compatibility, focused native regressions and necessary sanitizer/fuzz/release gates.

**Pass:** no undefined behavior, unsupported CPU instruction, data race or leak is introduced; explicit ownership and benchmark methodology; status and source-backed evidence updated. Nanoseconds apply only to tiny primitive microbenchmarks if measured; whole image pipelines require size-/codec-/hardware-specific metrics.

## 4. Cross-workstream non-negotiables

1. **Safety before speed:** exact output and memory safety precede performance. Never suppress safety checks or make unsupported 30 ms/100k ops/sec claims.
2. **Single authority per fact:** AGENTS = rules; `next.md` = current status/action; versioned `docs/` = contract and evidence; `skills.md` = optional execution recipes; GitHub source and test outputs = implementation truth.
3. **Design rigor without ceremony:** use SOLID/DRY, secure modular/hexagonal boundaries, reviewed algorithms/data structures and design patterns where they solve an actual changeability, ownership or testability problem. HLD and LLD must link source, tradeoffs, failure modes and tests; do not create a DDD/event-bus framework without requirement.
4. **Data privacy:** no permanent user image storage in target; isolate ephemeral workspace; redact filenames/image bytes/secrets/PII and cap telemetry cardinality.
5. **Release and governance:** one focused branch/PR; inspect baseline and dirty worktree; no merging, rewriting history, deploying, or retiring legacy without the user's explicit go/no-go. Tests claimed only when executed at the relevant commit; required CI is never waived.
6. **CA/MBA unit economics:** for each feature, state who benefits, cost per successful job (CPU/RAM/network/storage), expected support/maintenance burden, server vs browser work, direct variable cost, privacy risk and what customer evidence justifies paid infrastructure. No invented prices/revenue.
7. **Feature status vocabulary:** **DONE**, **IMPLEMENTED**, **VERIFIED (recorded)**, **PARTIAL**, **PLANNED**, **BLOCKED**, **LEGACY** and **RELEASED** have distinct meanings. A docs PR proves no runtime issue fixed.

## 5. Copy/paste Astra kickoff message

> Audit **current `main`**, not a historic PR branch. Read `AGENTS.md`, `next.md`, `skills.md`, `docs/ASTRA_ENGINEERING_START_HERE.md`, `docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md`, then only the chosen track's scoped AGENTS, authoritative contract, source and tests. The active backend is top-level `backend/` in Rust/Axum and native C is `imgengine/`; `imgengine-saas/backend/` is retained Python legacy. Do not add Python/FastAPI/Redis/PostgreSQL, a database/queue, or speculative infrastructure. Pick **one** bounded workstream: Track A Rust P13 M1.1, Track B UI M1.2 *after A is frozen*, Track C read-only C17 security/SDK review, or independent Track D infrastructure/P14 read-only audit with infra AGENTS and the infra threat register. Do not deploy or rotate real secrets without explicit separate approval. Before editing, give a short source-backed HLD/LLD/diff plan, threat/resource/performance analysis, exact acceptance criteria and change-scoped verification. Separate implemented/partial/planned and prior-recorded from freshly tested evidence. Keep the existing C ABI and P12 behavior. Work in one small PR, update canonical docs and `next.md`, report SHA and tests, and do not merge or deploy without explicit permission.

## 6. Remaining governance and release gates — not blockers to beginning code review

| Gate | What is needed before closing it |
| --- | --- |
| Actual release | Approved target host, secret/TLS/firewall, exact resource envelope, SLO/alerts, canary, recovery and rollback under [P14](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| Static public `.a` SDK | Separate packaging/ABI/install/license/dependency closure plan and external C/Rust consumer tests |
| License/provenance | Explicit owner decision on root license and third-party components; no invented license |
| Legacy services | Migration parity, owner/data disposition and rollback approval |
| Public accounts/paid plans | Real product demand, explicit durable-metadata/storage ADR, security/compliance and measured unit economics |
| Older open P12 PR #27 | Compare it with already merged main implementation; close/supersede only after review, **never blindly merge** |

**Starting Astra does NOT require implementing all future product features or completing the public release. It requires choosing a bounded track, current code truth, a verified contract, and explicit completion evidence.**
