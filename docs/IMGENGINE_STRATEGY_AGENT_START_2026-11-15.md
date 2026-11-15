# IMGENGINE — first move and Astra/Codex/human engineer operating brief

**Status: task brief for the next human-approved milestone, NOT permission to implement every feature or deploy (2026-11-15).** Exact reviewed main base `acc29bbf002890471ba0a63f01f75a6fbcaa49f3`; always refresh HEAD before using. The proposed strategic goal is one privacy-first platform with free browser tools and validated paid studio/API/offline offerings, while maintaining existing C17/Rust production-candidate safety.

## 1. Mandatory read order (short path)

1. `AGENTS.md`, `next.md`, `docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md`, `docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md`.
2. `docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md`, `docs/ASTRA_ENGINEERING_START_HERE.md`, `docs/PRODUCT_ROADMAP_BROWSER_FIRST.md`, `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md`.
3. **New strategy:** [product/business](IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md) + [native/API/Wasm/desktop performance architecture](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md).
4. Only for affected scope, the nested `imgengine/AGENTS.md`, `backend/AGENTS.md`, `imgengine-saas/frontend/AGENTS.md`, `infra/AGENTS.md`, and versioned ABI/API/P12/P13, performance, resource and release evidence.
5. Actual code/tests, current branch SHA/diff/dirty worktree and existing CI settings. Source and current contracts override outdated illustrative plans.

## 2. Proposed first decision (STRAT-00, **read-only audit + draft ADR**)

**Outcome:** a source-backed, human-reviewable priority decision whether and how IMGENGINE can shift frontend-first (local Rust/Wasm free tools) without leaving P13 ABI 2.3 unfinished forever or waiving C-A0 P0 safety/whole-host RSS. Do not rewrite all roadmaps from aspiration.

**Exact deliverables:**
- Reality map: currently working C17/Rust/Nuxt flows vs P13 partial, Wasm planned, public `.a` absent, desktop absent, billing absent, P14 blocked.
- One-page customer segment decision: first studio/application service-center workflow, 10-interview discovery protocol, time/rework/price measurement and free-vs-paid logic.
- ADR options: (A) complete Rust P13 first, then browser tools; (B) implement browser-local resize proof first while independently reviewing C-A0 and deferring P13 transport; (C) test browser-only no-Wasm prototype, then adopt Rust/Wasm only if measured; (D) stop and fix native high-risk issue first if verified. Report dependencies, risk and evidence.
- F0 Nuxt/Worker/Wasm contract (local image stays local), P-B0..P-B8 benchmark matrix and proposed measurable objectives rather than invented nanosecond/sub-ms promises.
- Dependency graph and smallest next three PRs; mark every constraint that needs human approval.
- Proposed documentation-only ADR/plan PR with exact changed paths, no C/Rust/Nuxt implementation and no status claims.

**Decision-quality gate:** reviewer can choose a priority option and authorize **one** next implementation PR; outcomes, expected test commands, legacy compatibility and risks are explicit.

## 3. Engineering workstreams AFTER strategy approval

**Track N — native engine safety and performance**: existing priority C-A0 exact caller tracing and metrics atomicity, deterministic test design, then separate minimal fix PR **only when problem confirmed**; independent whole-request RSS and pinned performance baseline; retain ABI and C17/C11/sanitizer/fuzz evidence. Owner: native Senior/Staff/Principal + security reviewer.

**Track B — free browser user journey**: Nuxt/Vue/TS + audited Tailwind/custom CSS decision; one actual Rust/Wasm Web Worker resize, using browser codecs where justified; no image upload; accessible UX; accepted mobile/error/memory tests; then crop/rotate/flip/conversion/compression as separate small PRs. Owner: frontend Mid/Senior + Staff integration + security/privacy review.

**Track P — customer/business discovery**: no code dependency; 10 direct studio/service-center interviews, workflow observations, willingness-to-pay pilots, monthly ledger and unit economics, choose one real repeated job. Owner: founder/product/CA-MBA lenses.

**Track R — paid Rust API**: NOT YET; after C safety/resource/host release gates and a customer contract, add separate explicit account/quotas/metering/billing/storage ADR and no public access without owner go/no-go. Owner: Rust Senior/Staff + SRE + finance/security.

**Track D — desktop**: NOT YET; prototype Tauri/native C only when a validated offline workflow warrants installer support and file-system access; business and security approval required.

## 4. Role assignments across seven engineer levels (no Intern)

| Perspective | First deliverable |
| --- | --- |
| Junior | Inventory actual source/test commands and a focused fixture or reproduced behavior |
| Mid-Level | Outline smallest credible one-tool implementation and user journey |
| Senior | HLD/LLD, error/cleanup, quality and security acceptance tests |
| Staff | Stable image-operation contracts across local UI / Rust / C ABI, migration and compatibility |
| Senior Staff | Dependency sequence, concurrency/memory/quality/release risks across workstreams |
| Principal | Choose alternatives on measured cost, portability, scalability and reliability |
| Distinguished | Challenge whether the platform has genuine long-term differentiation and unnecessary complexity |
| Security/SRE | Untrusted images, Wasm limits, C memory/FFI, resource abuse, host secret/TLS/rollback |
| CA/MBA/Product | Identify payer, measurable time saved, pricing hypotheses, gross margin and opportunity cost |

No claim of separate live “agents”; these are review perspectives. Senior/Principal/Security review should be independent where practical for high-stakes native unsafe, auth and deployment changes.

## 5. Paste this into Astra, Codex or another engineer's task

> **Task: IMGENGINE STRAT-00 (READ ONLY + DRAFT DOCUMENTATION PR).** Work at latest `main`, first inspect SHA/status/changed files. Read `AGENTS.md`, `next.md`, `docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md`, `docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md`, `docs/IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md`, `docs/IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md`, and `docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md`. Then consult applicable scoped AGENTS, current source, ABI/API and M0/P13/performance evidence.
>
> I want IMGENGINE to evolve into (1) reliable privacy-first free local Nuxt/Rust-Wasm image tools, (2) differentiated paid studio workflows only after customer validation, (3) a secure metered Rust/Axum -> C17 API after resource/abuse/host checks, and possibly (4) offline desktop and licensed C SDK after proven demand. Preserve strict C17, stable installed C ABI, Rust safe FFI and existing Nuxt flows. Do not add Python/FastAPI/Redis/PostgreSQL/new Python scripts, speculative persistence, queues, or unnecessary infrastructure.
>
> **Before code changes:** inspect the current `main` source and determine actual implementability; identify native C-A0 queue/metrics and aggregate RSS risks as hypotheses until reproduced, existing global native operation lock throughput implications, current Rust/Axum API and P12/P13 contracts, Wasm not-yet-implemented status, current benchmark protocol and P14 actual-VPS blockers. Propose **one** priority ADR reconciling browser-first work with old M1→M2 sequencing. Show an exact resize-Worker first-tool contract, a strict no-upload privacy test, and separate ns/µs kernel, prepared-render, full native, HTTP and browser workload benchmarks without claiming invented results.
>
> Define a first niche/customer discovery experiment with 10 studio/application-agent interviews. Recommend free vs paid by workflow value and actual compute, variable and support cost. Treat any example prices as **hypotheses**, not adopted. Keep desktop, SDK and paid public API as later gated milestones.
>
> Use proportional Junior, Mid-Level, Senior, Staff, Senior Staff, Principal, Distinguished, Security, SRE, CTO and CA/MBA review. Deliver a small **documentation-only draft PR** (ADR, source-backed decision/evidence matrix, next 3 bounded PR proposals, risk register), report exact SHA and link checks, and explicitly state tests NOT RUN. Do not merge, deploy, modify secrets/DNS, retire legacy, implement billing, spend money or change production without my separate explicit approval. Do not mark P13/C-A0/P14/SDK/desktop done.

## 6. After STRAT-00 is reviewed — first actual implementation PR

Choose **one** lane only:
- **Native C-A0 focused reproducer** if source trace raises credible live race/correctness risk; no unsafe optimization before proof.
- **Browser-local one-image resize** if user-centered F0 ADR permits it: Nuxt drag/drop -> local browser decode -> Worker-hosted validated Rust/Wasm resize -> local preview/export; typed errors; device/memory caps; no image-network requests; no secret; real browser tests.
- **Rust P13 M1.1** if continuity/contract needs exceed browser priority, preserving P12 2.2 compatibility.

**Done means scoped tests at exact SHA and acknowledged review**, not that the overall business, product, API, desktop or public launch is complete.

## 7. What to tell a human new hire

“Read the six start files and your scoped AGENTS, check current HEAD rather than trusting diagrams, implement one bounded outcome, create regression/negative tests, report actual measurements, maintain the ABI/private-image and no-new-Python policies, and never merge/deploy/spend or change monetization by assumption. Use the seven-level operating model for proportionate review and keep customer value and operating costs visible.”

**Future human roles to recruit only after demand and cash:** full-stack/frontend product engineer; native C/Rust systems engineer; designer/researcher; later SRE/security part-time and financial/legal specialists. Do not hire seven ranks or build a large team before there is evidence of a paying customer.
