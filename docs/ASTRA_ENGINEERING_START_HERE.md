# Astra start here — IMGENGINE frontend, Rust backend and C17 core

**Document purpose:** bounded engineering-agent handoff and audited start sequence. **Repository baseline updated:** `main` at `39e7099fbb31a2b879c4a9b2c22fca73de42419d` (PR #43 merged, 2026-11-15). M1.1 Rust/Axum/Nuxt server-proxy crop transport is **COMPLETE / recorded-verified** ([7/7 CI at PR #43 head](https://github.com/Rofikali/imgengine/actions/runs/43683324448)); **M1.2 frontend editor is the next bounded implementation**, using [M1.2 source-backed handoff](ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md). This is **not** a new test run or release certificate. Inspect HEAD/source and any local worktree before changes.

**Scope:** Nuxt/Vue/TypeScript frontend; Rust/Axum/Tokio backend, request supervisor and safe C FFI; portable C17 native SDK and ABI; security, privacy, observability, HLD/LLD, DSA, SOLID and performance; delivery and product economics. **Not authorized:** new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, database/queue/tooling replacements, unrelated infrastructure, public deployment or a new paid feature without explicit separate decision. Existing Python code and safety scripts remain retained legacy/tooling until reviewed retirement/replacement.

**Seven-level engineering review:** This start guide determines *which bounded task goes next*; the [Seven-Level Engineering Operating Model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) determines *how to review quality, security, technical strategy and CA/MBA economics*; the [Agent Milestone Handoff Template](AGENT_MILESTONE_HANDOFF_TEMPLATE.md) defines each agent's exact task and evidence report. Apply role lenses proportionally to risk; no extra autonomous agents or new tool permissions are implied. Keep the track ordering and review-only gates below unchanged.

**Future platform direction (proposal, not accepted task-order change):** For A–Z C17/Rust speed, free/paid image-product economics and possible desktop/SDK channels, use [platform/business strategy](IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md), [cross-channel performance plan](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md), and [STRAT-00 engineering-agent brief](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md). Agent should compare the existing P13-first sequence against browser-first alternatives in a **new reviewed ADR** before modifying the task-order policy. Maintain current P0 security, legacy, evidence and P14 release constraints.

## 1. Authority and document navigation

**Do not paste the whole documentation tree into one agent context** (92 Markdown files under `docs/` on this C17 audit branch); follow the selected scoped agent and current source. Read this concise index, root [AGENTS](../AGENTS.md), [next.md](../next.md), [skills](../skills.md), and [approved stack](TECHNOLOGY_AND_REPOSITORY_POLICY.md). Then read only the affected scoped AGENTS, source, versioned API/ABI and design/evidence documents. The source and exact-revision tests outrank old plans and aspirational HLD. A root `skills.md` is a playbook, **not automatically installed as a model skill**.

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
| Rust P5/P8/P12 endpoints/admission | **IMPLEMENTED / RECORDED** | Axum adapter, supervisor, safe FFI, P13 M1.1 user crop through FFI/admission/Axum and Nuxt server proxy (PR #43) | Browser-facing crop type/form/workflow/editor and native mid-operation cancellation not implemented |
| Nuxt 4/Vue/TS proxy/UI | **IMPLEMENTED / RECORDED** | Typed P5/P8/P12 routes, abort and object URL cleanup | P13 accessible manual crop UI, browser E2E and full production accessibility evidence |
| P13 crop | **PARTIAL; M1.1 DONE** | Native normalized geometry + ABI 2.3 and safe Rust adapter → Axum/admission → Nuxt same-origin proxy, PR #43 and exact-head CI | M1.2 browser typed form/workflow + accessible drag/zoom UI; M1.3 browser/native image parity |
| Native and API security | **CONTROLS EXIST; EVIDENCE PARTIAL** | Input validation, output caps, admission, private proxy, sanitizer/fuzz records | Aggregate decoded+scratch+output RSS under concurrency; actual decoder sandbox reachability; potential native metrics synchronization issue |
| Observability and HLD/LLD/design principles | **DOCUMENTED / PARTIAL PROOF** | Existing C telemetry modules, Rust lifecycle, old design-pattern examples and source-aligned matrix | Actual current operational metrics, synchronization, redaction, trace boundaries, source-specific invariants; do not call drawings certified architecture |
| Public release on low-cost VPS | **BLOCKED** | Local Docker/candidate and P14 runbook | Host-specific TLS, secrets, RSS, canary, recovery/rollback, operations, go/no-go |
| Legacy Python retirement | **DEFERRED** | Old backend still tracked, now separate from active Rust source | Proven parity, preservation/disposition of legacy data, rollback and explicit approval |
| Business SaaS, accounts, billing, tenancy | **PLANNED / UNDECIDED** | Strategy, unit-economics questions and optional product routes | Customer evidence, operating cost, approved durable metadata choice and threat model |

**Security findings are review hypotheses**, not reproduced vulnerabilities. **Performance goals are target-specific**, never universal nanosecond/microsecond image-processing promises.

## 3. Astra launch sequence — one bounded PR/milestone at a time

### Track A — P13 M1.1 transport contract: COMPLETED / MERGED

**M1.1 is not a new task for Astra.** [PR #43](https://github.com/Rofikali/imgengine/pull/43) merged C ABI 2.3 existing crop support into safe Rust FFI, supervisor/admission, Axum sheet endpoint and same-origin Nuxt **server proxy**; omission continues to select the P12 centered-fill path. [Seven GitHub CI jobs succeeded on the PR head](https://github.com/Rofikali/imgengine/actions/runs/43683324448). The PR records authoritative Ubuntu 24.04 verification at integrated `7ef95249a9068987338689eb6c665f5d876f8edc`. Do not call this a browser crop UI, M1.3 parity, native cancellation or production release. Historical pre-M1.1 task instructions are superseded by this status.

### Track B — Nuxt frontend next: P13 M1.2 interactive accessible UI

**Dependency satisfied:** Track A M1.1 transport contract merged in PR #43. Astra can **start M1.2 frontend implementation now**, grounded in the [M1.2 exact-source handoff](ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md). Recheck latest HEAD; avoid reimplementing the Rust transport.

**Start:** [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [frontend audit](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), `imgengine-saas/frontend/components/SheetRenderPanel.vue`, `composables/useSheetRenderWorkflow.ts`, `server/api/sheet-render.post.ts`, [P13](P13_INTERACTIVE_CROP_CONTRACT.md).

**Astra's deliverables:** fill the **still-missing browser `types/api.ts` `SheetRenderInput` optional crop contract** and `utils/render-form.ts` all-or-none serialization of the existing four `crop_*` fields; implement SheetRenderPanel / workflow interaction state for keyboard/touch/pointer drag, bounded zoom, reset, deterministic normalized crop, accessible labels/focus/live errors, cleanup and no exposed secret. Omitted crop still uses P12 centered-fill. Build one scoped UI PR with focused form/component and browser-a11y evidence. **This is a server-rendered P13 workflow**: no upload while dragging, but final Render uploads a photo; Rust/Wasm no-upload tools are future M2+. No claim that client abort cancels C.

**M1.2 pass:** typed all-or-none crop fields correctly reach the existing proxy only on user-selected crop; omission preserves P12; bounded preview geometry and basic server-result smoke checks, mobile/touch/keyboard/non-drag accessibility, object URL/image cleanup, meaningful error paths, Nuxt unit/typecheck/build plus relevant browser tests and exact-revision CI. **Comprehensive pixel parity and expanded P12 cross-boundary E2E evidence are M1.3**, not falsely marked complete here. Do not manually run full native ASan/fuzz for a pure UI change unless shared ABI/FFI changed; required CI is never waived.

> **2026-11-15 Track C deep source update:** Begin with the [principal C17 source/LLD/DSA/security audit](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [native README](../imgengine/README.md), and exact current source. First deliverable is C-A0 **read-only** concurrency reachability/test design for SPSC owner+thief and mixed-atomic metrics, plus aggregate resource accounting; do not prescribe a scheduler rewrite without a verified reproducible failure. Existing C11/C17/ABI tests and past fuzz results do not prove all interleavings. This is independent of the Rust P13 integration task.

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

> Audit latest `main` and dirty worktree first. **PR #43 merged M1.1; do not repeat it.** For the next implementation choose **Track B P13 M1.2 frontend editor**, and read the complete [M1.2 Astra task prompt](ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md), frontend `AGENTS.md`, P12/P13 and exact source. Preserve P12 omitted-crop centered fill, existing Nuxt-Rust contract and C ABI. Add the missing browser-facing typed optional crop fields, local preview, accessible pointer/keyboard/touch controls and cleanup, using a single scoped PR with real tests. Distinguish the local preview from the server-bound Render upload, and leave M1.3 parity and M2 Wasm as separate work. Track C C17 audit and Track D infra/P14 remain independent **review-only** options. No new Python/Redis/PostgreSQL, speculative services, new public billing, secret changes, CI bypass, merging or deployment without explicit separate approval. Report actual SHA/evidence and update `next.md`.

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
