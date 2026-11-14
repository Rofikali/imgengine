# IMGENGINE — Master Work Inventory and Next Actions

**Living status register.** Original inventory baseline: `f09eeae7cb8097a8c51188f64d6a0ab139ab09f2`; latest documentation/agent reconciliation reviewed against `main` at `bae6bda2e3db1f82b65865ba3d36ccb7b3c5bd93` on 2026-11-14 (PR #34 merged). Status is grounded in tracked source, versioned contracts and *dated* test evidence; this review did not rerun tests. GitHub cannot inspect uncommitted local worktrees. Refresh the baseline, evidence and status when implementation changes.

> **Binding future stack (ADR-TECH-001, 2026-11-14):** **Native C17 + Rust/Axum/Tokio backend + safe C FFI + existing Nuxt/Vue/TS frontend.** Do **not** introduce Python/FastAPI/Django/DRF/Flask, Celery, Redis, PostgreSQL or new Python scripts into the target product. Historical Python backend and existing verification scripts still exist; do not silently delete them or claim retirement. [Technology and folder policy](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) defines forbidden/approved tools, Rust ownership and staged folder migration. The target choice is **DECIDED**; physical restructuring and legacy retirement are **DEFERRED**.

**TECH-01 DECIDED:** new product backend is Rust/Axum, native C17, safe FFI and existing Nuxt; no new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL or Python tooling. **TECH-03 LEGACY:** old FastAPI/worker/Redis/PostgreSQL source remains, not an approved backend. **TECH-04 PARTIAL/OPEN:** 14 existing Python tooling scripts outside the legacy backend need individually verified Rust/shell replacements only when approved. **TECH-05 IN PROGRESS ON DRAFT PR:** Rust crates have moved to `backend/`; old Python folder has **not** moved and no release/CI pass is yet claimed; no migration, refactor or production release occurs in this docs PR. [Technology audit](docs/TECHNOLOGY_DOCUMENTATION_AUDIT_2026-11-14.md) contains the full evidence and [technology ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) governs future choices.

**TECH-05 implementation handoff — DRAFT SOURCE REFACTOR:** The existing Rust crates (Axum, supervisor, safe FFI and smoke) are being moved from `imgengine/rust/` to top-level `backend/` on a separate branch. CI and real build compatibility are **not yet proven**; do not claim this is complete on `main`. The C17 core, Nuxt frontend and legacy Python directory stay in place. See [Rust backend directory migration contract](docs/RUST_BACKEND_DIRECTORY_MIGRATION.md). Only merge after required Cargo/FFI/native/Docker/CI validation and review; P13 and P14 statuses remain unchanged.

## 0. Current status and next-action board

**Purpose of this file:** track **what is done, what is half-done, what has not started, what is blocked and exactly what happens next**. Do **not** copy full HLD/LLD, API specifications, agent rules or test logs here; link to canonical documents. The A–D inventory below retains finished work and long-term plans.

| Priority / ID | Area and status | Next bounded action and completion evidence | Governed by |
| --- | --- | --- | --- |
| **P0 / H01** | **P13 interactive crop — HALF IMPLEMENTED**: C ABI 2.3 done; Rust/Axum/Nuxt path and UI not done | M1.1 safe Rust FFI→HTTP→typed proxy; M1.2 keyboard/drag UI; M1.3 exact-image parity and P12 backward compatibility | [Native AGENTS](imgengine/AGENTS.md), [Frontend AGENTS](imgengine-saas/frontend/AGENTS.md), [P13 contract](docs/P13_INTERACTIVE_CROP_CONTRACT.md) |
| **P0 / FE-A03** | **Frontend release checks — OPEN**: P5/P8/P12 source implemented, production accessibility/E2E and current dependency disposition open | Verify keyboard/mobile and critical browser flows, dependency audit at release SHA, errors/response memory and production behavior | [Frontend AGENTS](imgengine-saas/frontend/AGENTS.md), [frontend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) |
| **P0 / BE-A02** | **Retained Python backend security — REVIEW OPEN** (not proven exploits) | Assess internal-token comparison, supplied request IDs, metrics network reachability, prod-secret rejection and log redaction; **do not** migrate or delete legacy automatically | [Legacy backend AGENTS](imgengine-saas/backend/AGENTS.md), [legacy audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) |
| **P0 / native audit** | **C/Rust native security — PARTIAL evidence** | Source-trace metrics concurrency, decoder isolation, aggregate RSS/CPU/disk limits and cancellation; independent tests only if affected | [Native AGENTS](imgengine/AGENTS.md), [native matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) |
| **P0 / H02** | **P14 public deployment — BLOCKED** despite local Docker evidence | Exact candidate and real-host resource/TLS/secrets/monitoring/canary/rollback evidence and human go/no-go | [release checklist](docs/MVP_RELEASE_CHECKLIST.md), [P14 runbook](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| **P1 / H03** | **Legacy retirement — DEFERRED**: Python FastAPI/Celery/Redis/Postgres remains tracked | Compare scoped parity, recovery, canary and rollback before explicit retirement decision; no new Python target features | [migration](docs/migration.md), [legacy contract](docs/SAAS_CONTRACT.md) |
| **P1 / O05** | **Public SDK/provenance — OPEN**: shared `.so` exists; externally installable public `.a` not established | Licensing/dependency review, static/public ABI parity design, external consumer compatibility evidence | [native matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) |
| **P2 / P01** | **Browser-first Rust/Wasm — PLANNED**: no first-party browser engine today | After M1, Astra-led ADR/threat model and bounded Worker prototype with mobile memory, size, latency and no-upload tests | [roadmap](docs/PRODUCT_ROADMAP_BROWSER_FIRST.md), [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) |
| **Governance** | **PRs #30–34 — MERGED documentation updates**; this board is not proof of release | Next milestone PR updates changed IDs, evidence/SHA and next action; retain local dirty worktrees. PR #27 remains open for separate reconciliation | [audit index](docs/README.md), [optional skills](skills.md) |

**File ownership:** [Root AGENTS](AGENTS.md) is repository policy; [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) governs Nuxt/Vue/TS and server-side proxy files; [backend AGENTS](imgengine-saas/backend/AGENTS.md) governs retained **Python** source only; [native AGENTS](imgengine/AGENTS.md) governs C17; [Rust backend AGENTS](backend/AGENTS.md) governs Rust/Axum at `backend/`. [skills.md](skills.md) contains optional task recipes, [change-scoped verification](docs/CHANGE_SCOPED_VERIFICATION.md) defines developer-loop tests, and `docs/` contains full contracts and audit evidence.

**Update rule:** Each merged implementation milestone changes its relevant F/H/O/P status **and** this board, adding the tested SHA, exact evidence, remaining gap and next step. Put long design notes and test logs in `docs/`; never change a status to “verified” from an AI summary, historical document, or documentation-only PR. A planned route is not a working customer feature.

## How to read this file

- **FINISHED (recorded):** implemented and supported by previous PR/test/contract evidence for its original scope; not necessarily production-ready.
- **IMPLEMENTED / RELEASE OPEN:** functioning source path; real-host/security/release evidence incomplete.
- **HALF IMPLEMENTED:** one layer exists but the end-to-end product/workflow is unfinished.
- **OPEN / BLOCKED:** an identified operational, release, governance, or security task awaiting evidence.
- **LEGACY / MIGRATION:** older code retained; do not assume it is today's target architecture or delete it abruptly.
- **NOT IMPLEMENTED / PLANNED:** roadmap intent only; do not claim customer availability.
- **UNVERIFIED / HISTORICAL:** sources disagree or only a local/old record exists; inspect before scheduling or removing.

**Sources of truth:** source and tests at a named revision → versioned API/ABI contract → revision-bound evidence → this task index → historical design notes. Existing `docs/PRD.md` (last updated 2026-08-17) still describes a FastAPI/Celery/Redis/Postgres asynchronous product and is **historical for that legacy path**, not the new browser-first Rust/Axum baseline.

## A. Finished or implemented with recorded validation

| ID | Area | Status and exact scope | Evidence / qualification |
| --- | --- | --- | --- |
| F01 | Native IMGENGINE C engine: decoding, resizing/layout/rendering, TurboJPEG, scalar/SIMD/AVX2, scheduler | **FINISHED (recorded core slice)** | `imgengine/`, `docs/ENGINE_SPEC.md`, `docs/SCHEDULER_CHARACTERIZATION_REPORT.md`; not all future codecs/tools |
| F02 | Native C build language M0 | **FINISHED / MERGED** strict C17 shipping, C11 whole-core regression, isolated C23 policy | PR [#29](https://github.com/Rofikali/imgengine/pull/29), `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md`; Ubuntu 24.04 Docker test evidence recorded |
| F03 | Stable public ABI v1 plus additive ABI 2.0–2.3 | **FINISHED (existing symbols)** | ABI export, layout and consumer evidence; do not break old consumers |
| F04 | Rust FFI, supervisor, lifecycle, bounded admission, Axum endpoints | **IMPLEMENTED / VALIDATED (recorded)** | `docs/RUST_FFI.md`, `docs/RUST_SUPERVISOR.md`, `docs/api.md`; commercial tenant service not complete |
| F05 | P8 FIT-only render with prior semantics | **FINISHED (slice)** | `docs/P8_PRODUCT_CONTRACT.md`; must remain separate from crop-to-fill |
| F06 | P9.3 bounded native JPEG destination allocation | **FINISHED (narrow control)** | `docs/P9_3_BOUNDED_OUTPUT_API_DESIGN.md`, PR #9; not a total memory bound |
| F07 | P11 deterministic physical-sheet geometry planner | **FINISHED (slice)** | PR [#13](https://github.com/Rofikali/imgengine/pull/13) |
| F08 | P12 centered-fill JPEG physical-sheet implementation and browser workflow | **IMPLEMENTED / LOCAL VALIDATION** | `docs/P12_PHYSICAL_SHEET_CONTRACT.md`, PRs #16–19; not a public production release |
| F09 | P13 C-side normalized crop geometry and additive ABI 2.3 | **FINISHED (native slice only)** | PRs [#24](https://github.com/Rofikali/imgengine/pull/24), [#25](https://github.com/Rofikali/imgengine/pull/25), [#26](https://github.com/Rofikali/imgengine/pull/26); no complete UI path |
| F10 | Nuxt 4/Vue/TypeScript frontend and typed same-origin proxy | **IMPLEMENTED** | `imgengine-saas/frontend/`, frontend `AGENTS.md`; does not include browser-local Wasm |
| F11 | Local Docker/Compose/Caddy hardening, loopback binding tests | **IMPLEMENTED / LOCAL EVIDENCE** | PRs #20–22, `docs/P12_DEPLOYMENT.md`; no real VPS claim |
| F12 | Native ABI/export checks, CTest, sanitizer, fuzz, Rust smoke/integration gates | **RECORDED PASS on prior revisions** | `imgengine/scripts/verify_linux.sh`, PR #29; do not misrepresent as rerun for this docs update |
| F13 | P7.5 grayscale dispatch and ABI diagnostic remediation in historical Git history | **MERGED HISTORIC SLICE** | PR #6 and `docs/P7_5_BRANCH_CONSOLIDATION.md`; separate dirty local worktree is a distinct issue |
| F14 | Master status/roadmap/testing-policy documentation | **MERGED** | PR [#30](https://github.com/Rofikali/imgengine/pull/30) added registers and updated `next.md` |

## B. Started but half implemented / incomplete customer journeys

| ID | Feature | Completed part | Outstanding part | Next action |
| --- | --- | --- | --- | --- |
| H01 | **P13 interactive user crop** | C crop geometry + ABI 2.3 and tests | Safe Rust FFI adapter, Axum contract, Nuxt proxy, drag/zoom/keyboard frontend, parity and lifecycle tests | **M1.1 → M1.2 → M1.3**, do not change ABI 2.2 centered-fill |
| H02 | **P12/P14 server-backed MVP launch** | Local JPEG sheet, private Compose, Caddy/Rust/Nuxt paths | Exact release candidate, real host capacity, TLS/DNS/firewall, monitoring, restart/drain/rollback, canary, go/no-go | Follow `docs/MVP_RELEASE_CHECKLIST.md` and `docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md` |
| H03 | **Legacy async → synchronous Rust product transition** | Rust/Axum synchronous path exists | Evaluate old feature/API parity, migration decisions, rollback, then controlled retirement | Do not delete Python/Celery/Redis/Postgres without explicit closure |
| H04 | **Security and tenancy** | API-key boundary, native limits, private proxy, Compose hardening | Paid tenants, API-key lifecycle, quotas, metering, abuse prevention, full memory accounting | Threat model per new paid boundary |
| H05 | **Operational observability** | Historical policies, logs, runbooks and local measures exist | Verify real-host alerting, log redaction, SLOs, on-call ownership and evidence | Separate release/security workstream |
| H06 | **Print/export feature breadth** | JPEG sheets with exact count and bounded geometry | Customer-controlled crop, reviewed presets, local sheets, PDF, ZIP, CMYK/prepress only after justified | Treat every added format as a separate contract |
| H07 | **Frontend experience** | Nuxt and working P12 UI | Accessible crop interaction, error/preview fidelity, broader image-tool UX | After transport contract stabilizes |
| H08 | **Repository documentation coherence** | Many detailed historical P7/P8/P10/P11/P12/P13 contracts | Old PRD and earlier AGENTS/README claims can be stale; label history and update canonical references | Documentation-only reconciliation, no automatic rewrite of history |

## C. Open, blocked, or uncertain tasks — do not mark complete yet

| ID | Finding | Evidence / action |
| --- | --- | --- |
| O01 | **Actual 1 GiB VPS capacity is unknown** | 544 MiB configured service ceilings are not whole-host peak RSS; P14 test required |
| O02 | **Production release approval not granted** | `docs/MVP_RELEASE_CHECKLIST.md` has outstanding boxes for digests/SBOM, host, canary, recovery, security |
| O03 | **Nuxt/Nitro advisories** | Prior audit reported moderate/low advisories; verify current lockfile/audit before risk acceptance |
| O04 | **Branch protection/required review governance** | At inspection `main` was unprotected; configure rules/required checks where permissions allow |
| O05 | **Root license/provenance question** | 2026-10-31 audit noted README MIT assertion but no root `LICENSE`; 2026-11-14 tree still lacks root LICENSE. Verify upstream/fork and third-party licenses before commercial release. Do not assume rights or invent license text |
| O06 | **Open older PR #27** | [P12 ABI 2.2 native sheet PR #27](https://github.com/Rofikali/imgengine/pull/27) remains open despite main containing P12; compare unique changes and decide close/supersede, not blind merge |
| O07 | **Separately dirty P7.5 local worktree** | User reported local uncommitted files. Remote GitHub cannot see them; preserve and inventory locally before cleanup |
| O08 | **C23 production enhancement** | Intentionally not implemented. C17 remains shipping; review only isolated C23 experiments with evidence |
| O09 | **Native cancellation** | Established C path is not mid-operation cancellable; response abandonment is not native cancellation. Revisit only if workload evidence justifies |
| O10 | **Decoded/aggregate memory, disk and workload ceilings** | Output byte caps alone insufficient; characterize real malicious and worst-case images |
| O11 | **Tooling economics** | Existing PR CI may run native suite on frontend docs; scoped local test policy exists but CI path-based optimization is **not implemented** |
| O12 | **Local vs remote history gaps** | Current GitHub inventory cannot claim exhaustive status of unpushed experiments/worktrees |

## D. Not implemented / future plans — comprehensive image-only product direction

| ID | Proposed area | Scope / caveat |
| --- | --- | --- |
| P01 | **Browser Worker + Rust/Wasm** | No first-party browser compute path today. Start **after M1**, Astra-led architecture review and bounded prototype |
| P02 | Browser crop/rotate/resize/convert | Local JPEG/PNG/WebP, browser codec fallback, no-upload assertion and real-device tests |
| P03 | Target-KB smart compression | 20 KB, 50 KB, 100 KB, custom sizes; report impossible constraints rather than promise universal 30 ms |
| P04 | Browser physical photo sheets | A4, 4×6, DPI, exact counts, margins and print fidelity; reuse contracts where applicable |
| P05 | Photo/application/visa presets | Versioned dimensions with official requirements, review owner, provenance, disclaimers |
| P06 | Signature/image document tools | Resize, background extraction, ID front/back layout; privacy-first without permanent image retention |
| P07 | Local batch workflows + ZIP | Bounded concurrent processing, partial failures, memory backpressure, per-image results |
| P08 | Advanced reusable presets | Save/organize dimensions, crop, DPI, format, compression and naming settings |
| P09 | Professional studio | Watermark, text, borders, masks, layers, non-destructive edit history, responsive render budgets |
| P10 | Paid B2B server API | Tenant identity, quotas, accounting, API auth, HMAC where appropriate, billing reconciliation |
| P11 | Advanced PDF/CMYK/prepress and very large sheets | Only if demand, licensing, accuracy, security and VPS cost justify |
| P12 | SEO/customer image routes | `/crop-image`, `/resize-image`, `/compress-image-to-20kb`, `/photo-sheet-maker`, `/studio` etc.; all proposed, not shipped |
| P13 | Tailwind/design system | Not proven installed; explicit design/dependency decision required |
| P14 | Studio Pro subscriptions/priority features | Pricing and paid tiers are **hypotheses**, not implemented transactions |
| P15 | Performance objectives | SIMD and native optimizations must be benchmarked; no universal sub-30-ms/60-FPS promise |
| P16 | Hosting/scale strategy | Minimize server processing for browser-local work; first prove retention, real demand, cost per job and margins |

For precise dependencies, routes, business model and milestones, use `docs/PRODUCT_ROADMAP_BROWSER_FIRST.md`.

## E. Next execution order

1. **Documentation reconciliation**: review this inventory and `skills.md` if adopted; preserve old worktrees and historical contracts.
2. **M1.1** native crop ABI 2.3 → Rust FFI → Axum → Nuxt proxy, with focused interface tests.
3. **M1.2** accessible manual crop UI, drag/zoom/reset/keyboard and safe local preview.
4. **M1.3** image parity, invalid cases, P12 backward compatibility; review and merge.
5. **M2** Astra-led Worker/Wasm design and small geometry prototype; first decide Rust-only, C17-Wasm, browser-native codec, or hybrid using measurements.
6. **M3–M5** local crop/resize/convert, target-KB compression, local photo sheets and presets, each in small PRs.
7. **Commercial workflow**: validate customer usefulness, local batch and paid studio convenience, then B2B only if economics work.
8. **Parallel P14**: actual VPS operational tests only on an explicitly approved candidate; public release stays blocked meanwhile.
9. **Repo hygiene**: review stale PR #27, reconcile uncommitted P7.5 locally, licensing provenance, dependency advisory disposition and branch protection.

## F. Tests, tokens and agent behavior

**Read `AGENTS.md`, affected directory `AGENTS.md`, `docs/CHANGE_SCOPED_VERIFICATION.md`, and optional workflow guidance in `skills.md`.**

- For documentation-only work: links/consistency/diff; **no manual C build**.
- For isolated Nuxt UI / Worker / Rust-Wasm: frontend tests, typecheck/build, browser/Worker/Wasm correctness, memory and no-upload security tests. **No manual full CMake, ASan, UBSan or native fuzz** unless shared native C/ABI/FFI boundary changed.
- For C, ABI, native codec/scheduler or unsafe FFI: focused native/ABI tests and necessary sanitizer/fuzz gates, escalating to full native verification.
- For release candidates: all required revision-bound release/security and real-host tests. Never suppress required CI to save tokens.
- To save model context: keep full command logs in files/artifacts and report only command, SHA, pass/fail and actionable errors; avoid duplicate manual runs already performed by CI.
- Astra reviews high-risk Wasm architecture, native/FFI changes, adversarial security and release gates; Terra/Sol implement bounded steps. An AI model is not a replacement for test evidence.
- Do not merge or deploy until the applicable review/go-no-go. Do not alter dirty local worktrees.

## G. Historical root next.md — preserved accurately

The **original 2026-08-17** `next.md` text is archived **verbatim** in [docs/HISTORICAL_NEXT_2026-08-17.md](docs/HISTORICAL_NEXT_2026-08-17.md), recovered from Git commit [a7be443](https://github.com/Rofikali/imgengine/blob/a7be44310311859f764cd4170728e0465d02a446/next.md). The previous empty “preserved verbatim” block was incorrect and is replaced by this source-backed reference. Original ideas about native sanitizer CI, observability, queues, database recovery and tenancy remain historical: first check later implementations and migration scope before reopening a task.

## H. Detailed audit handoffs — keep evidence out of this dashboard

The status IDs in the current board and the A–D inventory are the **actionable summary**. For findings, precise source locations and test-evidence limitations, read the relevant detailed audit rather than copying entire tables into `next.md`.

| Audit | What is established | What remains open |
| --- | --- | --- |
| [Native C/Rust audit](docs/NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) and [contract matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) | C17/C11 M0 evidence, public ABI 1.0–2.3, Rust FFI/admission/lifecycle, P7.4/P7.5 narrow fixes | Aggregate resource and isolation proof, possible metrics race review, public static SDK, portable binary/benchmark and release evidence |
| [Frontend/legacy backend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) | Nuxt 4 typed P5/P8/P12 UI/proxies; tracked FastAPI/Celery durable-job code is **legacy** | P13 browser-to-Rust crop, accessibility/E2E, current advisories, backend token/request-ID/metrics review, safe legacy migration |
| [Product roadmap](docs/PRODUCT_ROADMAP_BROWSER_FIRST.md) | Proposed future image-only user routes, batch/presets and pricing hypotheses | Browser Worker/Wasm and paid features are **not shipped** |
| [P14 operations checklist](docs/MVP_RELEASE_CHECKLIST.md) | Local-only Docker and release-candidate procedures | Actual VPS/TLS/secrets/canary/rollback and explicit approval |

**When closing an audit finding:** record a named SHA, implementation PR, review or test evidence (where applicable), owner/decision and remaining risk in its canonical audit/contract, then update only the corresponding summary row and priority in this file. A docs-only status review must not be represented as a production fix, security validation, or feature completion.
