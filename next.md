# IMGENGINE — Master Work Inventory and Next Actions

## LIVE BOARD — NOW / NEXT / LATER (authoritative task selection)

**Current GitHub checkpoint (verified 2026-11-16):** `main` `e26a9194a3c9f58059fcd81c8bd78cffd2199c33`. STRAT-00 **OWNER APPROVED**. [F0 sequencing PR #55](https://github.com/Rofikali/imgengine/pull/55) **MERGED** into that commit; its final PR-head CI **SUCCEEDED** for `073a16ab84901f67d78f5964622aed524adbc089`. [PR #54](https://github.com/Rofikali/imgengine/pull/54) and [CI-SCOPE-01 PR #53](https://github.com/Rofikali/imgengine/pull/53) also merged; all seven original CI jobs remain enabled. **F1 browser-local resize is now READY** (not implemented), and no free image-tool product is publicly launched. Recheck live GitHub at each new agent session—this is a dated checkpoint.

**Owner-selected four-product portfolio (PORT-00, 2026-11-16):** The product *names and directions* are approved: **IMGENGINE Free** (first launch **exactly five**: **Crop, Resize, Compress, Rotate, Convert**), **IMGENGINE Pro**, **IMGENGINE Studio**, **IMGENGINE Developer API**. The [canonical portfolio and phased product requirements](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [API usage/billing/capacity plan](docs/IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md), and [commercial readiness/finance gates](docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md) are **PORT-00 documentation in [draft PR #56](https://github.com/Rofikali/imgengine/pull/56)**; implementation **NOT STARTED**, CI for this PR requires its own exact-head verification; they do **not** authorize paid checkout, tenancy, new databases, SDKs, hosting costs or launch. The 2026-11-15 strategy and historical roadmap remain evidence, not active queues. Do not ask the owner to re-approve already selected product names.

| Lane | Milestone / factual state | Next bounded action | Completion / decision gate |
| --- | --- | --- | --- |
| **NOW — customer feature** | **F1 Free Resize — READY** | Build **one** Nuxt browser-local JPEG/PNG resize with preview, aspect lock, downloaded output, no image upload and device limits. One reviewed PR. | F0 merged; focused frontend/privacy/accessibility tests, current seven CI jobs, exact SHA; no deploy/merge without approval |
| **NOW — portfolio documentation** | **PORT-00 — DRAFT PR #56 / CI PENDING / NOT MERGED** | Review this four-product plan and link agent skills/task board. Do not modify image processing or introduce billing services in this documentation slice. | Docs source/link checks, PR CI exact SHA, owner review/merge; afterward mark PORT-00 merged, **not product released** |
| **NOW — independent safety** | **C-A0 — OPEN**, native concurrency/resource investigation | Prove or dismiss source-backed risks in a **separate** bounded C/Rust task. Do not reimplement existing merged code or block safe browser-only F1 without a verified dependency. | Native security/reproduction and exact-SHA CI evidence before any public server-backed release |
| **NOW — independent verification** | **P8-SIM-02 — MERGED PR #52; DEDICATED DOCKER WORKLOAD NOT VERIFIED HERE** | On an authorized Docker host run existing P8/P12/P13 workload, inspect OOM/cleanup, record SHA and actual host evidence. **Do not repeat implementation.** | Actual simulator report; ordinary CI green does not prove simulator or 1 GiB VPS fitness |
| **NEXT — Free five** | **F2a Crop → F2b Rotate → F2c Compress → F4 Convert — PLANNED** | One separate frontend/browser-local feature PR per step. Preserve F1 privacy, resource and accessibility guarantees. **No Flip or print sheet as sixth tool.** | Each real input/output/fidelity/browser/no-upload test, current CI and PR review |
| **PARALLEL OPTIONAL** | **F3 Rust/Wasm/Worker feasibility — PLANNED** | Compare native browser APIs, Worker and Rust/Wasm only with realistic benchmarks; **do not block F1–F4** on speculative Wasm. | Cold/warm latency, memory, bundle, compatibility, parity, security and explicit go/no-go |
| **NEXT — Free launch** | **F5 — PLANNED / NOT DEPLOYED** | Release a reliable **five-tool** website only after usability/privacy/hosting/ad/legal/security checks. Static/browser-local hosting may avoid Rust VPS. | Separate public launch/hosting/ads owner approval and measured runtime evidence |
| **LATER — IMGENGINE Pro** | **PRO-00/01/02 — PLANNED; NO SUBSCRIPTIONS** | Validate creator pain; trial local named recipes, bounded batch, naming/ZIP and ad-free experience. | Customer pilots, security/entitlement/account ADR, approved prices/payments/refunds before selling |
| **LATER — IMGENGINE Studio** | **STU-00/01/02 — PLANNED; EXISTING P12/P13 ≠ SHIPPED STUDIO** | Interview print/service shops; source and verify paper/photo presets; build accurate operator workflow in bounded PRs. | Real print/user validation, consent/privacy, current native/P14 gate if server-rendered; owner approved paid launch |
| **LATER — IMGENGINE Developer API** | **API-00…05 — PLANNED; NO PUBLIC METERED API** | Validate developers/capacity, then versioned sandbox, per-tenant keys/quotas, durable usage ledger, reconciliation, metered private beta and controlled launch. | C-A0/P14 host security/actual capacity, approved storage+payment ADR, invoice/credit audit, pricing/tax/abuse and owner go/no-go |
| **FUTURE OPTIONS** | **Native SDK/desktop/AI/extra Free features — DEFERRED** | Only after justified demand and a separate roadmap decision. | Licensing/provenance, platform tests and ROI evidence; not part of first five-tool launch |

**Permanent status/no-repeat rule:** At each start, after PR creation, CI result and the first active session following a merge, inspect fresh `main`, PRs and exact-head checks; record task ID, owner decision, current state (`READY → IN_PROGRESS → PR_OPEN/CI_PENDING → READY_FOR_REVIEW → MERGED`; `RELEASED` requires a real approved launch), PR URL, tested SHA and CI, merge SHA only after GitHub confirms it, unverified gates, residual risk and **one next approved task**. **A documentation PR is not code implementation, CI PASS is not a merge, and a merge is not a release.** Stale paragraphs below are historical and must never trigger duplicate work. See [root AGENTS](AGENTS.md), [Skill 18](skills.md), [portfolio skill](skills.md) and [agent continuation](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md). Human approval for merge, deploy, payment provider, secrets, DNS or spending remains mandatory.

---

## Historical work inventory and dated checkpoints (preserved; not the live task selector)

**CI-SCOPE-01 Phase A / IN REVIEW:** [Dry-run classifier](docs/CI_SCOPE_01_DRY_RUN.md) adds conservative Git path classification and tests within the existing frontend CI job. All seven jobs still run; no selective skipping, required-check or branch-protection change. Base: 758bedc24c9acd0ddde8a956210fd090d5bdb874. Exact tested SHA and CI results belong in the review PR. **M1.3 MERGED:** PR #49, merge 8fa2cc3faa651b99599b750978662cd85ce272fb; all seven [CI jobs passed](https://github.com/Rofikali/imgengine/actions/runs/43825725494) for head 77ac72044848676f201ad0ece3d76aa136842349. Remaining cross-browser/color, C-A0 and P14 evidence stays open.

**P8-SIM-02 (2026-11-16, PR CANDIDATE / NOT RUNTIME-VERIFIED):** [P8-SIM-02 sheet/crop + cgroup runbook](docs/P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md) adds P12 no-crop, P13 explicit-crop, malformed/duplicate/out-of-range and DPI-limit smoke to the existing P8 Docker fixture workload and includes cgroup `memory.peak`/`memory.events` reporting when supported. **P8-SIM-01 merged in PR #51** as `730d843c672c62ed55240d204adc1f2afe8412f4`. P8-SIM-02 still requires actual clean Docker test, no OOM, original P8 regression and new sheet/crop smoke at PR SHA; existing seven PR CI jobs do not run dedicated P8 simulation. M1.3 independent browser/pixel parity, C-A0 and P14 release remain separate. No deployment or merge implied.

**P8-SIM-01 (2026-11-16, DRAFT / NOT VERIFIED):** The isolated [P8 runtime reproducibility handoff](docs/P8_SIM_01_RUNTIME_REPRODUCIBILITY.md) proposes replacing historical P7-dev-image-inherited Rust/Nuxt simulation builds with maintained standalone runtime images and a separate non-root fixture image. It also switches Rust health from a 405 probe to `/readyz`, retains loopback-only Nuxt, and makes the workload harness validate Compose configuration before building. **P8-SIM-01 remains OPEN** until a clean-checkout Docker build, actual simulator workload, container-security inspections and exact-PR-head checks pass; ordinary seven-job PR CI does not itself exercise the P8 workload. **P8-SIM-02 remains separate** for P12/P13/EXIF and measured resource-envelope expansion. No change to production Compose, P14 host readiness, or M1.3 priority is asserted.

**Tooling review — root Makefile (2026-11-16; proposed, not yet merged):** [Root Makefile Engineering Guide](docs/ROOT_MAKEFILE_ENGINEERING_GUIDE.md) defines a safe repo-root command menu for scoped C17, Rust, Nuxt, browser and optional Linux/Docker full verification. It wraps existing tools; `make` defaults to help, `JOBS=2`, cleanup/full-verifier runs require explicit confirmation, no deploy/merge/paid infrastructure, and required GitHub CI is untouched. **Acceptance still requires `make self-check` (GNU Make help/dry-run/refusal checks) at actual PR SHA and normal PR CI**; no Make target was claimed run merely by documentation.
**Standing agent continuation / task-selection process (2026-11-16, docs-only):** [Milestone Continuation and New-Session Recovery](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md) is the permanent **read latest evidence → choose next READY task → one bounded PR → CI → review → checkpoint/resume** rule for Astra, Codex and human engineers. The agent should not ask the owner to repeat the same instructions at every milestone. **A stopped AI session cannot restart itself from documentation**; start a new session with the runbook's one-line prompt or configure a real approved event-driven coding runner separately. No autonomous merge/deployment or test bypass. **Verified main checkpoint:** [PR #47](https://github.com/Rofikali/imgengine/pull/47) merged as `de8b1708ba4e54cd219c93cc15f7923773ce8c9e`; [seven CI jobs passed](https://github.com/Rofikali/imgengine/actions/runs/43808582807) on PR head `b3aa42c2b89b0831a48448e8c4529cd67131ed75`. **M1.2 is MERGED; M1.3 parity is implemented for review below**. This does not certify public P14 release.

**Living status register.** **M1.2 MERGED (PR #47):** merge de8b1708ba4e54cd219c93cc15f7923773ce8c9e; seven CI jobs passed on PR head b3aa42c2b89b0831a48448e8c4529cd67131ed75 ([CI](https://github.com/Rofikali/imgengine/actions/runs/43808582807)). The accessible editor, typed crop transport and stored-raster EXIF preview contract are implemented. **M1.3 MERGED (PR #49):** bounded browser/native parity and P12 compatibility suite; see [evidence](docs/P13_M1_3_CROP_PARITY.md). C-A0 and P14 release remain open.

Original inventory baseline: `f09eeae7cb8097a8c51188f64d6a0ab139ab09f2`; latest repository-source-layout reconciliation reviewed against `main` at `e567616975476204c9279f476c28fb74e67512bc` on 2026-11-14 (PR #37 merged). Status is grounded in tracked source, versioned contracts and *dated* test evidence; this review did not rerun tests. GitHub cannot inspect uncommitted local worktrees. Refresh the baseline, evidence and status when implementation changes.

> **Binding future stack (ADR-TECH-001, 2026-11-14):** **Native C17 + Rust/Axum/Tokio backend + safe C FFI + existing Nuxt/Vue/TS frontend.** Do **not** introduce Python/FastAPI/Django/DRF/Flask, Celery, Redis, PostgreSQL or new Python scripts into the target product. Historical Python backend and existing verification scripts still exist; do not silently delete them or claim retirement. [Technology and folder policy](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) defines forbidden/approved tools, Rust ownership and staged folder migration. The target choice is **DECIDED**; Rust crates are **MOVED / MERGED** under top-level `backend/` by PR #37; the legacy Python directory move/retirement is **DEFERRED**.

**TECH-01 DECIDED:** no new Python/FastAPI/Celery/Redis/PostgreSQL or new Python tooling for the target product. **TECH-03 LEGACY:** retained Python backend exists under `imgengine-saas/backend/`. **TECH-04 OPEN:** existing Python verification/release scripts must not be removed before tested Rust/shell replacement. **TECH-05 DONE / MERGED:** [PR #37](https://github.com/Rofikali/imgengine/pull/37) moved Rust API/FFI/supervisor under top-level `backend/`; all [seven CI jobs passed](https://github.com/Rofikali/imgengine/actions/runs/43618598771) on PR head `4ab5a65fcde411fd77dde8eb36aea09dce7915c3`. This is **not** a public production release. Source-backed handoff: [Astra engineering start guide](docs/ASTRA_ENGINEERING_START_HERE.md) and [technology ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md).

**INFRA AUDIT (2026-11-15):** [Infra security, operations and economics audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) covers all 18 tracked active infra files at baseline `6838ed73403235450da310f0e7fa6f8a8d6f8bf0`. [Scoped infra AGENTS](infra/AGENTS.md) owns Docker/Caddy/Compose rules; P7 local dev, P8 simulation and P14 public deployment are **distinct**. Static production Compose hardening is implemented; actual-VPS proof, host-file secrets, total RSS, observability/abuse controls and rollback remain **OPEN**, while the older P8 simulation has a source build-cache/toolchain provenance gap. No live configuration was changed or tested in this docs audit.

**C17 CORE SOURCE AUDIT (2026-11-15):** [Principal LLD/DSA/security/performance report](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) at source baseline `ff5dd06755069d170d97c9ab323a2fa52915aa68`. **C-A0 P0 REVIEW OPEN:** work-stealing may concurrently consume an SPSC ring alongside its owner; native metrics mix atomic counters with ordinary reads/max/min; prove caller reachability and exactly-once/error behavior before a code fix. **P0 RESOURCE EVIDENCE:** whole-request decoded/canvas/stb-copy/scratch/output/Rust memory under concurrency. **P1:** opt-in seccomp vs actual runtime, ARM64/portable build, public static `.a` SDK, internal PDF error handling. No runtime reproduction, merge or safety remediation was performed by this docs audit.

**Engineering-agent operating rules (docs-only):** The [Seven-Level Engineering Operating Model](docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) and [Agent Milestone Handoff Template](docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md) define proportionate Junior-to-Distinguished review lenses, security/SRE/CA-MBA business gates, and an exact-SHA evidence handoff for Astra/Codex. These are **governance guidance**, not proof that P13, C-A0 or P14 changed status. Existing P0 priorities, human approval and release blockers below remain unchanged.

**STRAT-00 strategy candidate (documentation only, not a completed milestone):** Read the [Platform Product & Business Strategy](docs/IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md), [C17/Rust/Browser/Server/Desktop Performance Plan](docs/IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md) and [Agent First Move](docs/IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md). These propose a browser-first free-tools product, differentiated paid Studio workflow, and **later** metered Rust/C API, native SDK and desktop after customer evidence. A priority ADR must explicitly reconcile existing M1→M2 sequencing; C-A0 P0 native review, aggregate RSS and P14 safety/release blockers remain active. No code, launch, pricing or status is declared DONE.

**Permanent agent testing/log-output policy (2026-11-15; docs only):** [CI-first minimal-AI-context log policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md) is the short operational rule for Astra/Codex/engineers across **every milestone**: focused local tests, **full existing GitHub CI on PRs**, summary-only exact-SHA results, failed-step-only excerpts, and no duplicate native/sanitizer/fuzz runs merely to fill the AI context. Do not conflate model tokens with GitHub runner minutes; no required test, release or approval gate is weakened. Agents should not require the owner to repeat this instruction for each milestone.

**CI cost/token-scope proposal (2026-11-15; documentation-only):** The [Test Lanes & Agent Budget runbook](docs/TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md) and [CI-SCOPE-01 Astra handoff](docs/ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md) distinguish targeted developer testing from existing **seven-job PR CI** and full release/ABI/security verification. Agents may immediately avoid **manual** native rebuild/sanitizer/fuzz duplication for isolated Nuxt/Worker work, while still testing touched HTTP/proxy contracts. **CI-SCOPE-01 (PHASE A IN REVIEW; SELECTIVE EXECUTION NOT IMPLEMENTED):** dry-run conservative path classifier, fail-closed selected-job gate and independently reviewed workflow/branch-rule rollout; currently all seven CI jobs still run for every PR. No release or required check is waived.

## 0. Current status and next-action board

**Purpose of this file:** track **what is done, what is half-done, what has not started, what is blocked and exactly what happens next**. Do **not** copy full HLD/LLD, API specifications, agent rules or test logs here; link to canonical documents. The A–D inventory below retains finished work and long-term plans.

| Priority / ID | Area and status | Next bounded action and completion evidence | Governed by |
| --- | --- | --- | --- |
| **P0 / H01** | **P13 interactive crop — PARTIAL**: C ABI 2.3 + M1.1 Rust FFI, Axum/admission and Nuxt **server proxy** merged in PR #43; Ubuntu verifier recorded at `7ef95249a9068987338689eb6c665f5d876f8edc`, seven CI jobs passed at PR head `3f7819c16a793500f0668dfdb91af961072646d6`; **M1.2 editor merged in PR #47; M1.3 parity review remains open** | Review M1.3 parity evidence and required CI; retain P14 release gates | [Astra M1.2 handoff](docs/ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md), [Frontend AGENTS](imgengine-saas/frontend/AGENTS.md), [P13](docs/P13_INTERACTIVE_CROP_CONTRACT.md) |
| **P0 / FE-A03** | **Frontend release checks — OPEN**: P5/P8/P12 source implemented, production accessibility/E2E and current dependency disposition open | Verify keyboard/mobile and critical browser flows, dependency audit at release SHA, errors/response memory and production behavior | [Frontend AGENTS](imgengine-saas/frontend/AGENTS.md), [frontend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) |
| **P0 / BE-A02** | **Retained Python backend security — REVIEW OPEN** (not proven exploits) | Assess internal-token comparison, supplied request IDs, metrics network reachability, prod-secret rejection and log redaction; **do not** migrate or delete legacy automatically | [Legacy backend AGENTS](imgengine-saas/backend/AGENTS.md), [legacy audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) |
| **P0 / C-A0** | **C17 owner/thief SPSC + metrics concurrency — REVIEW OPEN**: native worker steals call the owner queue's SPSC pop; metrics mix atomic and ordinary accesses, caller reachability incomplete | Source-trace 2–8 worker interleavings and metric readers; propose deterministic exactly-once/concurrency tests **before** approving any C fix | [Principal source audit](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [native AGENTS](imgengine/AGENTS.md) |
| **P0 / native audit** | **C/Rust native security — PARTIAL evidence** | Source-trace metrics concurrency, decoder isolation, aggregate RSS/CPU/disk limits and cancellation; independent tests only if affected | [Native AGENTS](imgengine/AGENTS.md), [native matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) |
| **P0 / H02** | **P14 public deployment — BLOCKED** despite local Docker evidence | Exact candidate and real-host resource/TLS/secrets/monitoring/canary/rollback evidence and human go/no-go | [release checklist](docs/MVP_RELEASE_CHECKLIST.md), [P14 runbook](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| **P0 / INFRA-01/02/03** | **P14 infrastructure controls configured; RELEASE BLOCKED** | Verify host-file Compose secrets permissions and non-root access, public abuse/rate control, real-VPS aggregate RSS/CPU/tmpfs and service SLOs. [Infrastructure audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) distinguishes implemented config from evidence. | [Infra AGENTS](infra/AGENTS.md), [P14 runbook](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| **P1 / INFRA-04/05/06/07/08/09/10** | **INFRA REVIEW OPEN**: P8 image provenance/development-toolchain inheritance, static-checker blind spots, secret process-env handoff, Caddy readiness/alerts, release supply chain and rollback | Astra Track D source-first HLD/LLD/security review, then separate bounded PRs only after evidence/approval; never add new Python, skip CI or deploy | [Infra audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [infra README](infra/README.md) |
| **P1 / H03** | **Legacy retirement — DEFERRED**: Python FastAPI/Celery/Redis/Postgres remains tracked | Compare scoped parity, recovery, canary and rollback before explicit retirement decision; no new Python target features | [migration](docs/migration.md), [legacy contract](docs/SAAS_CONTRACT.md) |
| **P1 / O05** | **Public SDK/provenance — OPEN**: shared `.so` exists; externally installable public `.a` not established | Licensing/dependency review, static/public ABI parity design, external consumer compatibility evidence | [native matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) |
| **P2 / P01** | **Browser-first Rust/Wasm — PLANNED**: no first-party browser engine today | After M1, Astra-led ADR/threat model and bounded Worker prototype with mobile memory, size, latency and no-upload tests | [roadmap](docs/PRODUCT_ROADMAP_BROWSER_FIRST.md), [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) |
| **Governance** | **PRs #30–37 — MERGED; PR #37 is verified structural relocation, not production**; this board is not proof of release | Next milestone PR updates changed IDs, evidence/SHA and next action; retain local dirty worktrees. PR #27 remains open for separate reconciliation | [audit index](docs/README.md), [optional skills](skills.md) |

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
| F15 | Rust backend directory relocation (`backend/`) | **DONE / MERGED / 7-of-7 CI verified** | PR [#37](https://github.com/Rofikali/imgengine/pull/37), [exact PR-head verification](https://github.com/Rofikali/imgengine/actions/runs/43618598771); not a live VPS release |

## B. Started but half implemented / incomplete customer journeys

| ID | Feature | Completed part | Outstanding part | Next action |
| --- | --- | --- | --- | --- |
| H01 | **P13 interactive user crop** | C crop geometry + ABI 2.3 plus M1.1 Rust FFI, Axum, bounded-admission, and Nuxt-proxy transport; Ubuntu 24.04 verification passed at `7ef95249a9068987338689eb6c665f5d876f8edc` | Drag/zoom/keyboard frontend, browser image-parity evidence, and final P12 compatibility review | **M1.2 → M1.3**, do not change ABI 2.2 centered-fill |
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
2. **M1.1 — COMPLETE / VERIFIED at `7ef95249a9068987338689eb6c665f5d876f8edc`**: native crop ABI 2.3 → Rust FFI → Axum → bounded admission → Nuxt proxy, with focused interface tests and full Ubuntu 24.04 verification.
3. **M1.2 — MERGED, PR #47**: [source-backed frontend handoff](docs/ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md), browser `SheetRenderInput` + `createSheetRenderForm` now carry optional **all-four or none** `crop_*` fields; accessible manual crop UI, drag/zoom/reset/keyboard/touch, safe local preview, upload only on intentional Render.
4. **M1.3** browser/native image parity, invalid cases, P12 backward compatibility; review and merge.
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
