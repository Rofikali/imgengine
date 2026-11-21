# IMGENGINE — Engineering Skills and Workflow Recipes


**Skill — IMGENGINE Pro native-first milestone (owner choice 2026-11-21, review candidate):** Start at [features/pro/STATUS.md](features/pro/STATUS.md) for evidence and [features/pro/README.md](features/pro/README.md) for A–Z Pro features. For C17/Rust-only work, use [features/pro/NATIVE_TESTING.md](features/pro/NATIVE_TESTING.md), `features/pro/compose.native-test.yml` and the optional `.github/workflows/pro-native-smoke.yml`; do not implement the Pro frontend yet. One small `feat/pro/<milestone>` PR, focused tests and owner review/merge; update status, next.md and affected docs in the same PR. No fabricated PASS, Pro feature completion, paid launch, release tag or deployment from documentation alone.


**F5 Free release engineering / Cloudflare runbook (2026-11-21):** [Read-only manual release audit](.github/workflows/free-v1-release-preflight.yml), [Cloudflare Direct Upload operator procedure](docs/releases/free/v1.0.0/CLOUDFLARE_PAGES_RELEASE_PLAYBOOK.md) and [draft notes](docs/releases/free/v1.0.0/DRAFT_RELEASE_NOTES.md). Select immutable reviewed source; execute pinned Ubuntu24 Docker, frozen lockfile zero-moderate release audit and separate fail-closed `verify:free-publication-ready`, full exact-head CI, 84 local and 200 static browser cases, CSP/no-upload allowlist, detached artifact digest, real legal/support owner contacts and provider TLS/rollback evidence. **Only after full owner go/no-go** tag `free-v1.0.0` and perform a separate explicit Cloudflare Pages Direct Upload. No tag, deployment or cloud account action is accomplished by a preparation PR; do not auto-Git-deploy `main`, invent a URL or weaken a check.

**F5-DOCSYNC status reconciliation recipe (2026-11-21):** [PR #91](https://github.com/Rofikali/imgengine/pull/91) is **MERGED** and the GitHub milestone generator runs; [scheduled run 44840635463](https://github.com/Rofikali/imgengine/actions/runs/44840635463) succeeded, but Actions was denied permission to create a review PR. [Generated snapshot PR #92](https://github.com/Rofikali/imgengine/pull/92) was opened manually. F5-DOCSYNC-02 adds a tested retry for identical bot-branch content with no PR, and requires a visible failure until owner-enabled Actions PR creation is available. Follow [runbook](docs/engineering/MILESTONE_AUTOMATION.md); this remains review-only and does not automatically edit unrelated durable policies, approve security risk, merge or release.

**F5-SUPPLY-05 reproducible remediation recipe (2026-11-20):** [#89](https://github.com/Rofikali/imgengine/pull/89) merged as `6c9e5ff7846d6d4e3b4ec63d62b19ac0d3af3008` with [post-merge CI SUCCESS](https://github.com/Rofikali/imgengine/actions/runs/44771824270), enforcing but not satisfying the zero-moderate Free release audit. Start with the [bounded dependency handoff](docs/releases/free/v1.0.0/dependency-remediation-handoff.md) and current [next.md](next.md); check upstream transitive version ranges before editing the Nuxt/Nitro graph. Use pinned Node/pnpm inside approved Ubuntu 24.04 Docker to regenerate `pnpm-lock.yaml`, verify frozen install, `verify:free-release-dependencies`, full required exact-head CI and all local/Free static four-browser and privacy checks. No manual lockfile edits, arbitrary major overrides, audit suppression, fabricated PASS, merging or public release. Update impacted docs and distinguish current checkpoint from historical failures.

**F5-HOST-HDR-01 release engineering recipe:** For Free static, generate root `_headers` from rendered HTML and actual inline script SHA-256 hashes; validate one wildcard provider rule, Cloudflare 2,000-char limit, CSP `script-src` no `unsafe-inline`/eval, `connect-src 'self'`, frame/form/object restrictions and exact byte-for-byte artifact audit. Ensure Playwright test server applies generated headers and all browser/privacy tests pass. Do not deploy, merge automatically, mark legal notices approved, override publication guard, or assert live Cloudflare TLS/analytics/rollback without owner-reviewed evidence.

**F5-TRUST-01 review recipe (2026-11-20):** Check [Free release evidence](docs/releases/free/v1.0.0/evidence-matrix.md), static-only route contract, all-nine-route browser tests and independent `verify:free-publication-ready` fail-closed result. Do not use guessed identity/contact details, treat withdrawn picomatch GHSA as universal safety, or claim compliance with phased DPDP rules without an actual owner/legal review. Existing CI and native/browser gates remain mandatory.

**Skill — versioned release dossier (F5-RELEASE-DOCS-01 merged; F5-STATIC-01 in review):** Start at [docs/releases/README.md](docs/releases/README.md), use [shared templates](docs/releases/templates/README.template.md) and copy only the relevant version-specific contract/evidence/runbook into a new product folder; do not fork the product repository or create empty future versions. For Free v1.0.0 use [static-only dossier](docs/releases/free/v1.0.0/README.md). Keep an immutable post-freeze artifact manifest **separate** from the source-controlled release plan (avoid SHA self-reference); record PASS only for actual release-candidate execution. Preserve existing Docker-only tooling, required CI, privacy/legal/operations signoffs and owner-only production authority.

**CI-SCOPE-02 reusable workflow (review candidate, 2026-11-19):** Use [CI selective verification and aggregate gate policy](docs/CI_SCOPE_02_SELECTION_GATE.md), `scripts/ci-scope-select.mjs` and gate truth-table tests when reviewing PR CI. Only actual changed code plus transitive consumers are tested on narrow PRs; native/C ABI/FFI, CI/infra/security/lockfile and unknown changes fail closed to full; every main push/release remains full. Required selected jobs must **actually succeed** and skipped jobs do not count as passing tests. Confirm the owner has configured the `CI verification gate` as a required main-branch check before treating selective PR green as mergeable. Do not auto-change branch protection or merge.

**Full-discipline review + next-job keyword (2026-11-19):** Use [seven-level engineering plus CISSP/CCIE/top-level Linux/SRE/CA/MBA/management operating model](docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) in place of loose “Principal/Staff” shorthand for current milestone briefs. **“Go ahead” means: inspect current GitHub + [next.md](next.md), continue the highest-priority eligible unfinished job, repair open PRs and blockers first, make a bounded tested PR, stop before owner-only merge/deploy.** Do not restart from an old task or create duplicate PRs. For F2c see [PR #72](https://github.com/Rofikali/imgengine/pull/72) and its [12-gate contract](docs/FRONTEND_F2C_CI_QUALITY_GATES.md).

**Frontend Excellence F6 playbook (planning only):** Read [F6 program](docs/IMGENGINE_FRONTEND_EXCELLENCE_PROGRAM_F6.md), [acceptance/evidence matrix](docs/IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md), scoped [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) and [next.md](next.md) before a future post-Free PR. After actual Free release, start with F6-00 exact-SHA UX/a11y/SEO/perf/privacy baseline; one bounded milestone per reviewable PR, focused tests plus required CI, no upload or premature Rust/Wasm rewrite. This documentation does not change the current Free work queue.

**Five-product documentation recipe (proposed, 2026-11-19):** For release/branching/SDK/Desktop planning, consult [ADR-PORTFOLIO-005](docs/ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md) and the [product release and compatibility matrix](docs/IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md) **after** root/scoped AGENTS and live next.md. Keep a short-lived branch, exact-SHA review, separate ABI/API/app versions, no assumed SDK or desktop implementation, and no release-state promotion from documentation alone. This does not displace F2c Compress or the seven existing required CI jobs.

**2026-11-19 status and post-merge rule:** [Live `next.md`](next.md) is the sole task selector. F1 #57, F2a #59, F2b #62, F2c #72 and F4 #76 are **MERGED**; Free v1 is **not released** and F5 release review is next. [PR #65](https://github.com/Rofikali/imgengine/pull/65) **MERGED** only a [temporary frontend non-production picomatch waiver](docs/FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) (expires **2026-12-02 UTC**, not a patch) plus three native runner labels. [Real Codex Agent Skill/lean AGENTS PR #63](https://github.com/Rofikali/imgengine/pull/63) is **DRAFT, not yet on main**; this `skills.md` remains a manual recipe list. **Every milestone PR proposes truthful `next.md` updates and affected contracts; the next authorized small PR after an external merge must reconcile actual merge SHA and stale README/skills/AGENTS/docs index facts.** No automatic merge/deploy or fabricated CI. [Runbook](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md).

**Status:** Optional task playbook; not an executable skill, installed Codex skill, or automatic GitHub Copilot instruction. The tracked repository did not previously contain a root `skills.md` on 2026-11-14. **`AGENTS.md` and nested `AGENTS.md` are authoritative for agent rules**; this file provides reusable *how-to* recipes without replacing them. If using a tool-specific skill installer, follow that tool's supported skill format and path rather than assuming this markdown is auto-loaded.

> **Approved technology constraint:** All new backend work uses Rust/Axum/Tokio and safe C17 FFI, not Python/FastAPI/Django/DRF/Flask, Celery, Redis, PostgreSQL or Python-based scripts. Existing legacy code is retained solely for migration/maintenance; do not rewrite or delete it without a separate approved gate. First read the [binding technology/folder ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), then the [Rust-only scoped AGENTS](backend/AGENTS.md) for active backend tasks. Proposals for durable paid-product metadata require user approval of a new storage ADR; no automatic database adoption.

**Scope reminder:** [Binding stack ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) and [source audit](docs/TECHNOLOGY_DOCUMENTATION_AUDIT_2026-11-14.md) apply to all future work. Existing Python scripts for ABI, release and Compose checks are **present and must not be deleted without verified Rust/shell replacements**. Folder renames need separately approved path/CI/Docker/Cargo/CMake migration tests; documentation changes alone are not permission.

## Authority and startup

**Astra-ready navigation:** [docs/ASTRA_ENGINEERING_START_HERE.md](docs/ASTRA_ENGINEERING_START_HERE.md) defines the bounded frontend, Rust backend, and C17 native review/implementation tracks. Choose a single milestone; it is not an installed skill, a test report, or permission to introduce new services.

Read, in this order:
1. `AGENTS.md` and relevant nested `AGENTS.md` (rules and boundaries).
2. `next.md` (current state and one active milestone).
3. The corresponding versioned contract in `docs/` and latest implementation/tests.
4. `docs/CHANGE_SCOPED_VERIFICATION.md` (which tests are justified).
5. This file for task-specific recipes.

Stop when the repository differs from the declared SHA or critical evidence cannot be verified. Preserve separate dirty worktrees; never silently modify them. No push/merge/deploy without explicit approval for the relevant action.

## Skill 1 — Scoped feature implementation

- Identify user outcome, current API/ABI contract, affected directories, acceptance criteria and rollback.
- Inspect only relevant files first; avoid dumping unrelated C source or test logs into the model context.
- Create one small feature branch; implement one layer/milestone at a time.
- Keep public contracts versioned; use typed boundaries and explicit errors.
- Run focused tests appropriate to touched files. Summarize pass/fail, revision and meaningful failure excerpts; link full CI logs instead of pasting them.
- Update `next.md` status and affected documentation in the same PR.

## Skill 2 — Nuxt frontend and browser privacy

- Prefer accessible components, keyboard and touch controls, typed API adapters, and no secrets in public config.
- For local image jobs, never upload image bytes or analytics containing image data without explicit authorization. Revoke object URLs, release buffers, handle tab/unmount cancellation and limit input dimensions.
- Run frontend tests, typecheck/build and relevant browser/E2E tests.
- **Do not manually run native CMake/sanitizer/fuzz for pure UI changes**. Required GitHub CI remains in force.

## Skill 3 — Rust/WebAssembly Worker

- Start with architecture and threat review; prototype a bounded single operation before introducing a full codec port.
- Compare browser-native codecs, Rust/Wasm geometry, portable C17-Wasm and hybrid execution. Evaluate memory allocations/copies, device compatibility, bundle and startup cost, SIMD/thread limitations, licensing, color/orientation parity and CSP/isolation.
- Test malformed images and input sizes, Worker crash/cancellation, clean resource release, no network upload, and mobile memory pressure.
- Run focused Wasm/Rust/Worker/browser checks; do not manually run native C tests unless shared C/ABI/FFI files or ownership changed.
- Never promise zero-copy, universal 30 ms compression, or 60 FPS without reproducible measurements.

## Skill 4 — Native C17, ABI and Rust FFI

- Shipping portable core is strict C17. Preserve installed C11 consumer behavior; C23 is isolated and capability-gated only.
- Version additive ABI changes, freeze public layouts, explicitly document allocation/release and unsafe Rust invariants.
- Run impacted ABI symbols/struct-layout/consumer tests, native regressions, C11/C17 builds when relevant, ASan/UBSan/fuzz where affected, and required authoritative verification before a native release.
- Do not replace the proven C scheduler without an evidence-backed architecture decision.

## Skill 5 — Security and deployment

- Treat every upload as hostile; validate at every trust boundary. Bound bytes, decoded pixels, intermediate buffers, canvas, CPU time, disk, concurrency and output.
- Keep processing ephemeral and logs redacted. No client secrets or unapproved permanent retention.
- Test deployment controls only in an approved environment. Real VPS inventory, TLS/firewall, monitoring, rollback, canary and ownership decisions are mandatory for public release.
- No readiness claim solely from Docker Desktop, local screenshot, or CI.

## Skill 6 — Product, CA/MBA and commercial review

- Define the customer, job-to-be-done, demand evidence, product scope, implementation/operating cost and commercial risk.
- Prefer free browser-local crop/resize/compress/basic print tools; monetize bounded batch operations, reusable presets and business workflows when willingness to pay is demonstrated.
- Calculate contribution margin as revenue net of direct variable costs; separately record fixed overhead, acquisition cost, retention, refunds, taxes and support burden.
- Do not introduce billing, tenant databases, queues or microservices before their customer and threat-model requirements are established.

## Model handoff — efficient tokens, preserved safety

| Task | Suggested model / effort | Validation |
| --- | --- | --- |
| Routine docs, frontend UI, well-specified PR | Terra / Sol, medium | Focused diff and component tests |
| New Rust/Wasm architecture, browser memory model | Astra, high | ADR + threat model, prototype parity |
| ABI/unsafe FFI/security-sensitive review | Astra, high | Independent review + relevant native gates |
| Release readiness | Human owner + independent reviewer | Required exact-revision CI and real-host evidence |

Keep one compact handoff report per milestone: baseline SHA, changed files, commands, results, unresolved risks, documentation updates, and PR link. Avoid rerunning unrelated suites manually.

## Native C/Rust audit evidence recipe (linked)

Use [Native/Rust Contract-to-Evidence Matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) together with the [81-file document map](docs/NATIVE_C_RUST_DOCS_MAP.md) and [dated source audit](docs/NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md). For every native/Rust review identify the exact public or private entry point, ownership, memory/CPU budget, possible concurrent readers/writers, observability unit/redaction, test evidence and any uncertainty. Do not invent production readiness or require a full native build for docs-only changes.

## Skill 7 — Nuxt frontend and retained Python backend audit

Read the [frontend and legacy backend source audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), scoped [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) and [backend AGENTS](imgengine-saas/backend/AGENTS.md). Note that the latter governs *legacy Python*, while current Rust/Axum/FFI is under `backend/` and [backend AGENTS](backend/AGENTS.md); shared C ABI rules remain under [native AGENTS](imgengine/AGENTS.md).

1. Identify whether the task changes a Nuxt component, same-origin proxy, current Rust contract, or retained Python service. Do not mix these generations.
2. Trace exactly which P5, P8 or P12 contract is affected; reject unsupported P13 crop, PDF, job, billing or account claims.
3. Verify server-only secret handling, bounded upload/response memory, safe typed errors, abort semantics, object URL cleanup, accessibility and current advisories.
4. For legacy work, trace FastAPI auth, DB job transitions, queue/retry, internal token, metrics exposure, retention, backup and rollback; do not retire it without approval.
5. Run focused frontend Vitest/typecheck/build, browser/a11y checks or scoped legacy Python tests **only when those boundaries change**. Avoid manual C rebuild/sanitizers/fuzz for docs/UI-only work; mandatory CI and release gates remain.
6. Report status as implemented, recorded-verified, partial, planned or legacy with source citations and open business/security cost. Update `next.md` in the same PR.

## Skill 8 — Maintain the living work register without duplication

1. Read [next.md](next.md) **current board first** and locate the named F/H/O/P, FE-A or BE-A item. Confirm relevant root and nested AGENTS rules and exact contract/source at current head before work starts.
2. **Keep `next.md` as status + owner role + next action + acceptance evidence.** Full test logs, HLD/LLD, threat models, API schemas, old RFCs and long audit tables stay in their canonical `docs/` files.
3. Classify each claim as **planned, implemented, recorded-verified, partial, blocked, legacy or released**. Source presence is not a test pass; a merged documentation PR is not feature completion. Record exact SHA, related PR, what was tested, what was not, and residual risk.
4. The [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) governs Nuxt/Vue/TS; the [backend AGENTS](imgengine-saas/backend/AGENTS.md) governs retained Python. Rust/Axum/FFI is under `backend/` and [backend AGENTS](backend/AGENTS.md); the C core follows [native AGENTS](imgengine/AGENTS.md). Do not conflate them.
5. Preserve [original Aug 2026 backlog](docs/HISTORICAL_NEXT_2026-08-17.md) as historical provenance; do not automatically re-open its completed sanitizer/native work or legacy Redis/Postgres tasks.
6. For a docs-only update, check changed-file scope and relative links, **no manual C rebuild/ASan/UBSan/fuzz**. Do not weaken existing CI, native release requirements or required deployment evidence.

## Maintenance rules
- Treat `next.md` as live state; do not duplicate the entire backlog here.
- If a workflow becomes obsolete, mark it superseded rather than silently deleting historical intent.
- Separate "planned", "implemented", "validated" and "production released" in every claim.

## Skill 9 — Native C/Rust documentation and correctness audit

Use [the dated source-backed audit](docs/NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) and [complete documentation map](docs/NATIVE_C_RUST_DOCS_MAP.md). Restrict this recipe to the native C17 core, public `.so`/future `.a`, FFI wrapper, Rust supervisor, admission, security, observability, HLD/LLD, DSA and algorithms. Do not review browser/Wasm or frontend code when the milestone excludes them.

1. Inventory actual source and tests at an exact SHA; mark every statement **implemented, verified with revision, partial, planned, legacy, historical or open risk**.
2. Compare versioned installed headers and exported C symbols with Rust FFI and contracts. Identify ownership, allocation/release, struct sizes, error mapping, thread-safety and cancellation limitations.
3. For security, map every untrusted-byte path, process/OS sandbox claims, aggregate memory/buffer quotas, input validation and cleanup. Do not call a suspected data race a proven exploit without concurrent reachability/test evidence.
4. For native observability, distinguish C-level encoder/decoder/scheduler measurements from Rust admission/HTTP; document atomicity, cardinality, privacy, instrumentation overhead and deterministic timing regions.
5. For HLD, LLD, SOLID/DRY, hexagonal/DDD and patterns, record **source-backed actual usages** rather than mandating all patterns. For queues, allocators, and image kernels state algorithmic complexity, synchronization invariants and trade-offs.
6. For performance, separately measure tiny dispatch, kernel, prepared rendering, decode, encode, and end-to-end; keep scalar image-quality/bitwise correctness and security gates. No unverified nanosecond or 100K ops/s claim.
7. Finish with a concise finding register: ID, status, severity, cited file, residual uncertainty, owner, next verification. Update `next.md`. For docs-only audit: Markdown links/consistency/diff only, **no manual native build**, and preserve required GitHub CI.

## Skill 10 — Infrastructure security, P14 operations and cost review

Read [infra AGENTS](infra/AGENTS.md), [infra source audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [infra environment guide](infra/README.md), [P14 runbook](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) and [release checklist](docs/MVP_RELEASE_CHECKLIST.md). Distinguish P7 dev, P8 simulated VPS, P12 local disposable rehearsal, P14 production configuration and **actual** real-host evidence.

- Examine Caddy public 80/443 boundary, Nuxt proxy, internal Rust service, mounted credential host-file owner/group/mode, runtime UID/capabilities, resource ceilings, TLS renewal, alerts and rollback. Never print keys or user images.
- Docker Compose **ignores** long-syntax uid/gid/mode on file-backed secrets; verify actual host and container metadata before public release. Treat a rendered policy JSON check as static validation only.
- Review the **existing** Python Compose checker without deleting it or introducing more Python. A proposed Rust/shell replacement requires exact negative-test and CI parity in a separate approved PR.
- Reject unfounded claims that P8 simulation proves a clean checkout or a 1 GiB VPS; investigate its mutable dev-image and missing tracked offline registry directory separately from production.
- Report evidence at a named SHA/image digest, success/failure, real-host vs simulation, resource/abuse and customer-cost risk. Do not authorize deploy/DNS/secret/CI change or relax required gates to save tokens.

## Skill 11 — C17 principal source audit, LLD, DSA, design and secure-code review

**Read:** [source-backed C17 A–Z audit](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [native README](imgengine/README.md), [native AGENTS](imgengine/AGENTS.md), [versioned public ABI](docs/ABI.md), [prior C/Rust evidence](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md), and [next.md](next.md). Current source and exact-SHA tests take precedence over historical HLD/LLD/GoF sketches.

1. **Scope the real compiled C target:** public vs internal ABI, 12 versioned exported symbols, common CMake sources, plugins, C11/C17 profiles, CPU build flags and installability of .so vs public .a. Do not claim the static public SDK exists from the internal static target alone.
2. **C-A0 first, review-only:** trace owner SPSC queue pop versus stealing worker calls and producer/consumer identities, task state and exactly-once conditions. Separately trace metrics atomic counter writes, plain min/max reads/writes and core-index bounds; identify real concurrent callers. Design a deterministic multiple-worker stress/repro test **before** a fix PR.
3. **Audit memory and hostile data:** map compressed input ownership, TurboJPEG/stb staging/copies, slab/arena, canvas/scratch, output bound, total RSS and Rust request concurrency. Confirm active seccomp build/profile and permitted syscall boundary without inventing a separate decoder sandbox.
4. **DSA and design patterns:** identify real ring buffers, MPMC, slab free-list, bump allocator, O(P²) rectangle validation, fixed-point bilinear O(W×H), jump-table Strategy/vtable adapter. Report actual invariants/complexity, not pretend every GoF pattern/DDD/event bus is implemented.
5. **Portability, performance and product economics:** confirm x86-64 portable baseline and actual ARM64 build before redistributing; use pinned benchmarks with quality oracles, p95/p99 and peak memory before claiming microseconds. Quantify the effect on host capacity and cost per valid image, not a made-up revenue number.
6. **Change-scoped testing:** C/FFI/security fixes require focused deterministic regression, ABI C/Rust consumer, concurrency/sanitizer checks and mandatory CI. For docs-only audit, use Markdown source references, links and scope; **do not manually run full C builds or fuzz**.
7. End with IDs/status/priority/caller path/evidence/uncertainty/lowest-cost verification; update the single relevant next.md item. Never mix the current Rust backend with legacy Python, and never merge/deploy/retire services without explicit authorization.

## Skill 12 — Seven-level engineering, security and CA/MBA agent handoff

Use the [Seven-Level Engineering Operating Model](docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) to review each meaningful IMGENGINE change from Junior, Mid-Level, Senior, Staff, Senior Staff, Principal and Distinguished perspectives **proportionally to risk**, together with security/privacy, SRE, product/UX, CA/finance and MBA/management. These are review perspectives, not seven independent AI agents or an authorization to act autonomously.

1. Open the [Agent Milestone Handoff Template](docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md); name one exact next.md task ID, mode, affected directory, current SHA and acceptance matrix. Read only the scoped AGENTS, source and contracts that task needs.
2. For simple changes, do the shortest sufficient review. For C ABI/FFI, scheduler, decoder, auth, trust boundaries or deployment, produce a source-backed HLD/LLD, ownership/concurrency/DSA checks, threat model, compatibility risks and rollback plan before editing.
3. Make no new Python/FastAPI/Redis/PostgreSQL or speculative DB/broker additions. Do not confuse legacy services, old CI or static Compose settings with the selected Rust/C17 product's implementation and release evidence.
4. Run the change-scoped tests and preserve mandatory CI. Separate **NOT RUN**, historical evidence and tests executed at the exact revision. Benchmark correctness and total resources before claiming performance savings.
5. Include variable cost per successful job, customer value, opportunity cost, support burden, privacy and infrastructure assumptions; mark unknowns explicitly and never fabricate sales/profits.
6. Deliver the template's report with changed paths, SHA, status, risks, user decisions and one next milestone. Do not merge, deploy, rotate secrets, spend money or retire legacy without separate explicit authorization.

## Skill 13 — Product portfolio, C17/Rust performance and sustainable business planning

Use the proposed [platform/product/business strategy](docs/IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md), [C17/Rust/Wasm/API/desktop performance architecture](docs/IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md), [STRAT-00 first-move agent prompt](docs/IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md), [seven-level engineering model](docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) and current `next.md`.

1. Define **one** buyer/user job before adding image tools; test pricing against genuine repeated time-saving or server costs, not a hypothetical valuation.
2. Classify free browser-local tools vs paid studio workflows vs later metered public API/SDK/desktop. Default free tools do not upload image bytes; backend fallbacks require explicit opt-in. No database/billing/auth/queue without separate ADR.
3. Make all speed claims workload-specific: pure geometry vs prepared pixel kernels vs decode+render+encode vs Rust HTTP vs browser/desktop start-to-finish. Include input, revision, CPU, memory, cold/warm and correctness. No universal microseconds.
4. First source-trace the native C-A0 queue/metrics concerns, public ABI operation lock and aggregate RSS; review suspected security issues before optimizing concurrency. Do not treat suspected findings as confirmed.
5. For a frontend-first prioritization change, draft a reviewed ADR reconciling existing P13 M1 and browser Wasm M2 sequencing, preserving security/release gates and stable C ABI.
6. Delegate STRAT-00 as read-only architecture/product/business discovery with a **draft docs PR**; use the task-specific agent template to decide the next one bounded implementation PR. Keep source evidence, scope, human approval and costs explicit.

## Skill 14 — Change-scoped testing, safe CI lanes and model-context efficiency

**Standing CI-first output policy:** [CI-first minimal-AI-context log policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md). Test suites run in local shell/CI **outside model text**; only show job name, outcome, exact SHA, evidence URL and bounded failed-step excerpt. All green logs stay in CI/local artifacts. Reading full success logs is not a requirement to confirm a green check. No skipped/unknown test is PASS. This rule applies automatically as repository guidance to any contributor that reads root AGENTS; a new AI tool must be instructed to read those rules.

**Required reading for Astra/Codex/humans:** [Test Lanes, Milestone Gates & Agent Budget](docs/TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md), [Change-Scoped Verification](docs/CHANGE_SCOPED_VERIFICATION.md), and the separate [CI-SCOPE-01 safe implementation handoff](docs/ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md).

1. Before any test, inspect HEAD/base and complete PR diff **plus staged, unstaged and untracked changes**. Assign docs/UI/Wasm/HTTP proxy/Rust API/FFI/native/infra/unknown risk class; list downstream contracts.
2. **L0 local loop:** run the smallest safe focused test set; for pure Nuxt or future isolated browser-Wasm changes, do **not** manually rebuild C17/native sanitizers/fuzz/Ubuntu Docker. HTTP form/proxy edits still require relevant Rust/proxy integration; C/ABI/unsafe FFI still require native/security checks.
3. **L1 PR:** the inspected `.github/workflows/ci.yml` currently runs **seven unconditional jobs on every PR**. Allow required CI and consume summarized statuses rather than manually re-running every job and injecting full passing logs into the model context.
4. **L2 release/native safety:** full relevant C17/C11/ABI/ASan/UBSan/fuzz/Rust/container tests and exact candidate/host release evidence when risk warrants. No prior-commit or skipped-check PASS claim.
5. **CI-first minimal-context execution:** apply the [dedicated policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md); retain local exit codes and full logs off-chat. After PR publication, retrieve only the exact-SHA CI check conclusions/URLs (no full successful logs); inspect just the failed step if red. Summarize <=12 lines, including PASS/FAIL/NOT RUN/PENDING/SKIPPED and actionable next action. Failure is never ignored to save tokens.
6. Automatic selective GitHub job execution remains **CI-SCOPE-01 PROPOSED**, with a dry-run path classifier, fail-closed unknown changes, always-on required aggregate gate, branch-rule review and separate human-approved CI PR. Do not touch `.github/workflows/ci.yml`, skip tests, merge or deploy just to reduce compute.

## Skill 15 — Autonomous milestone selection and safe agent resume

**Standing runbook:** [Milestone Continuation and New-Session Recovery](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md). Use for Astra, Codex, human engineers and every new task; do not require the owner to paste old milestone reports and write a new prompt each time.

1. Inspect latest `main`, current worktree, merged/open PRs, exact-SHA CI and `next.md`. Reconcile stale status before deciding "what next"; don't rewrite history or uncommitted work.
2. Select highest-priority **READY**, dependency-satisfied, authorized task. Respect P0 security blockers, versioned API/ABI, user/customer value and cross-workstream boundaries.
3. Deliver one bounded PR with focused checks and existing required GitHub CI, following [minimal-context test policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md); do not fetch successful full logs.
4. CI PASS means `READY_FOR_REVIEW`, **not automatically MERGED**. A confirmed merge event moves status to `MERGED`. Select a next independent authorized task only if the runner is still active and owner approval boundaries permit.
5. When stopped, write a short checkpoint with main SHA, task/PR, tested SHA, risks, next eligible task and owner decision. **Do not pretend Markdown instructions launch a new model session.** Resume with the runbook's one-line bootstrap, or use a separately configured and approved coding-agent runner/event automation.
6. Never automatically merge, deploy, change DNS/secrets, buy services, modify public ABI incompatibly or invent new product milestones. Escalate unresolved high-risk decisions and stop affected work.

## Skill 16 — Root Makefile: consistent safe command menu

Use the [root Makefile](Makefile) as the entry point, plus [its engineering guide](docs/ROOT_MAKEFILE_ENGINEERING_GUIDE.md). Unlike the existing C-only `imgengine/Makefile`, root `make` covers Nuxt, Rust and C17, but delegates all real build/test behavior to CMake, Cargo, pnpm, Playwright and `verify_linux.sh`.

1. **Default safe:** `make` or `make help` prints the command menu; `make self-check` safely verifies menu/dry-run/refusal behavior without building; `make status` shows local branch/SHA/dirty paths. Do not run `make clean` without `CONFIRM_CLEAN=YES`.
2. **Nuxt-only:** `make frontend-test`, `make frontend-check`, `make frontend-browser` (requires built Nuxt and Playwright browser). Live HTTP flow: `make frontend-browser-live` only when services/fixtures/credentials are ready.
3. **Rust:** `make rust-test RUST_CRATE=imgengine-api` (builds/link native C prerequisite); `make rust-fmt-check` for formatting. Pick the affected crate; do not automatically run all native sanitizers/fuzz for Rust API-only changes.
4. **Native:** `make native-test`; for affected C11/unsafe ABI/security risk add `make native-c11`, `make native-sanitize`, `make native-fuzz`, and full gate `make verify-linux CONFIRM_FULL=YES` if warranted. Bounded `JOBS=2` by default.
5. **Docker/full:** `make verify-docker CONFIRM_FULL=YES` is an intentionally expensive Ubuntu 24.04 native verifier, **not** equivalent to all seven GitHub CI jobs. Keep existing full CI/release gates.
6. **Evidence:** follow [minimal-AI-context log policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md); full-verifier logs stay under ignored `build/make-logs/`. Never claim a target passed without running it, or infer public release approval.

## Skill 17 — P8 simulation: P12/P13 HTTP smoke, resources and truthful evidence

For local Ubuntu Docker engine P8 simulation changes, follow [P8-SIM-02 suite and resource guide](docs/P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md), [infra AGENTS](infra/AGENTS.md), and [P8-SIM-01 runtime handoff](docs/P8_SIM_01_RUNTIME_REPRODUCIBILITY.md).

1. Select only a disposable isolated checkout and ensure non-production secrets/fixtures. Run the existing `./infra/scripts/run-p8-vps-sim.ps1 -Build` harness where PowerShell and Docker are available.
2. Confirm original P8 FIT tests pass and `workload.sheet_crop` reports P12 omitted crop, P13 selected crop, negative-input status checks, JPEG dimensions and executed placement count. This does **not** prove per-pixel parity or EXIF; use the separate [M1.3 suite](docs/P13_M1_3_CROP_PARITY.md).
3. Record exact SHA, image IDs, health, cgroup version, actual cgroup `memory.peak` and `memory.events` when available, restart/OOM state, JSON report and workspace cleanup. `unavailable` is NOT a measured zero or PASS.
4. Use concise logs following [CI-first policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md); required seven GitHub jobs remain mandatory, yet they do **not** replace the dedicated P8 simulator.
5. Do not merge/deploy, change production Compose or use real secrets. Explicit owner approval and P14 actual-host security/cost gates still apply.

## Skill 18 — Evidence-backed milestone status, owner decision, and Go ahead

**New permanent completion rule:** Before a milestone handoff, follow the [one-source-of-truth and conditional doc-impact transaction](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md#mandatory-documentation-impact-transaction-at-every-passed-milestone-2026-11-19). Update truthful `next.md`, and affected AGENTS/skills/READMEs/contracts/CI/security/CA-MBA docs; mark N/A for unrelated docs. Actual CI green is READY_FOR_REVIEW; only owner merge gives MERGED. Keep the next `go ahead` job and no unsolicited push/merge/deploy.


**Binding status rule:** [root AGENTS](AGENTS.md) and [Milestone Continuation and New-Session Recovery](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md). `next.md` begins with the live **NOW / NEXT / LATER** queue. All older sections are retained as historical detail; a stale old row is *never* a reason to rebuild already merged work.

1. On **every agent session start** and before interpreting “go ahead”, inspect the current `main` SHA, open/closed/merged PR state, exact head SHA, and required GitHub CI. Compare against the live queue and verify the requested milestone has not already been merged. Work from the real checked-out source, never an outdated chat report or previous-SHA test result.
2. A strategy choice may be **OWNER_APPROVED** without a matching implementation. `READY` means prerequisites and necessary owner decisions are satisfied. Use `IN_PROGRESS` once coding actually begins, `PR_OPEN/CI_PENDING` for submitted changes awaiting checks, and `READY_FOR_REVIEW` after required CI passes at exact PR SHA. **Only a verified GitHub merge may become `MERGED`; only a real, separately approved launch may become `RELEASED`.** `BLOCKED`, `FAILED` and `DEFERRED` are not passing statuses.
3. During the bounded PR, update **only its current task** in `next.md` with task ID, priority, decision, factual progress, PR URL (when known), exact tested SHA, CI status/link, risk, next action. Keep long HLD, code and test logs in canonical docs or CI, not `next.md`.
4. If GitHub merges a PR after the agent session ends, **the next agent session** must reconcile `next.md` against GitHub and write `MERGED`, merge SHA, CI evidence, and next eligible milestone in a new authorized docs/implementation PR. **Do not claim that a closed PR automatically updates the contents of `main`.** Preserve historical paragraphs and the original verification dates; mark old text historical instead of silently deleting it.
5. For **STRAT-00**, the owner already approved *browser-first sequencing* on 2026-11-16. The active documentation/reconciliation milestone **F0** creates and merges the binding priority ADR/queue; after it merges, start bounded **F1** Nuxt/Vue/TypeScript browser-local resize with no upload, no new Python or DB. Rust/Wasm comes later only after an explicit measured browser Worker/ADR decision; do **not** postpone the free resize merely to implement a speculative Wasm engine. C-A0 native security and P14 actual-host safety remain independent hard gates for relevant work.
6. At each stop give the owner **one short checkpoint**, e.g. `F1 | READY_FOR_REVIEW | PR #NN | head SHA ... | CI 7/7 PASS | next: owner PR review | merge/deploy: NOT AUTHORIZED`. On a later “Go ahead”, start the next **approved + unblocked** task; if the current PR is still awaiting approval, do not open a duplicate PR or relaunch identical work.

## Skill 19 — Four-product portfolio: Free five, Pro, Studio and metered Developer API

**Always read first:** [Root AGENTS](AGENTS.md), [live NOW/NEXT/LATER queue](next.md), [accepted F0 sequencing](docs/F0_BROWSER_FIRST_SEQUENCING_ADR.md), [canonical four-product scope](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [commercial/CA-MBA release gates](docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md) and, for API work, [tenant/usage/billing/capacity blueprint](docs/IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md). This is **a task recipe, not an installed executable agent skill or permission to ship all four products**.

1. **Start with actual GitHub state.** Reconcile `main`, current PRs, CI, dirty worktrees and the live board per Skill 18. Product names/direction are owner-selected, but paid/ads/billing/provider/hosting implementation and release are *not*. F0 #55, F1 #57, F2a #59, F2b #62, F2c #72 and F4 #76 are merged; F5 release review is the next Free gate. Do not start with an old M1.2/M2 plan or repeat merged P8/P13/F0.
2. **Keep the Free launch five-tool bounded:** Crop, Resize, Compress, Rotate, Convert only. All five feature PRs are merged; F5 public launch remains separately approved. Flip, sheets, watermark and extra tools are *later*; no speculative feature creep.
3. **Browser privacy/quality gate:** default no image uploads, analytics without bytes/filenames, no external fallback, strict decoded-pixel/byte/length bounds, object-URL cleanup, output accuracy, formats/EXIF/metadata/alpha disclosures, keyboard/mobile tests and observable errors. Validate network capture on failure paths. Browser image processing may use native browser APIs; Worker or Rust/Wasm adoption only if F3 benchmark/security gate proves benefit.
4. **Pro discovery and features:** after real Free usage, interview creators and measure minutes saved. Only then implement schema-versioned local recipes (PRO-01), bounded batch/ZIP (PRO-02), integrated workflow/entitlement (later). Check file import/ZIP safety, memory, account/license/refund decisions; no paid checkout or cloud DB by assumption.
5. **Studio discovery and features:** measure shop/photo-agent time, paper/ink waste, reprints and repeat usage. Source/version physical print presets, operator proofs, DPI/mm/copy-count parity and consent. Reuse existing P12/P13 *only within their real server-backed contract*; do not call it a launched Studio product. Public C17/Rust requires C-A0 and P14 evidence and explicit release approval.
6. **Developer API steps (API-00…05):** interviews/capacity baseline → stable private sandbox with client auth/quotas → tenant isolation/admission → approved durable ledger/idempotency/payment architecture → private beta/cost reconciliation → separate paid launch approval. Do not expose the shared Nuxt→Rust key to customer apps; do not implement monetization in C17; no invented endpoints/formats/SLAs.
7. **CA/MBA economics:** record gross versus recognized revenue versus cash received versus profit, ad-eligible RPM versus pageviews, paid conversion/churn/refunds, cost per successful API transformation, payment processing, bandwidth, support, taxes and fixed expenses. Rates ₹199 Pro / ₹399 Studio are *hypotheses*, not approved commercial prices; API weights and prices await benchmarks and customers.
8. **Security and architecture:** C17 native pure execution and ABI compatibility; Rust/Axum policy and safe FFI; Nuxt browser-local UI; no new Python/FastAPI/Celery/Redis/PostgreSQL or unapproved databases, background queues, microservices, public API or payments. Any durable commercial metadata requires explicit storage/provider ADR, recovery/security/finance signoff and human approval.
9. **PR acceptance:** one scoped feature per PR with exact revision, changed boundaries, focused unit/browser/ABI tests as appropriate, current seven required GitHub CI conclusions, real privacy/capacity evidence (no guessed speed/profit), and short rollback plan. Update **only factual** task states in `next.md`. A merged design document is **not** feature implementation, subscription acceptance, real host capacity or released product.
10. **After merge:** next agent session reconciles the merge SHA, CI and next READY task. If Pro/Studio/API is still deferred, continue the five-tool Free queue rather than endlessly writing infrastructure or pricing plans. No autonomous merge, live advertising, charge, deployment, DNS/secret change or cloud spend.

## Skill 20 — Cost-efficient Astra/Codex resume and safe infra experiments

**Canonical narrow audit:** [Infrastructure experiments and AI-context cost audit](docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md). This is a workflow recipe, **not** a token-usage meter, an installed automation or approval to weaken CI.

1. Read **only short live** [`next.md`](next.md), current GitHub main/PR/CI summary, changed files and required scoped AGENTS/contract. [`next-history.md`](next-history.md) preserves old evidence; read it only for an exact historical question. Automatically preloaded context may be outside the agent's control.
2. Do **one** bounded PR and one focused set of local tests; don't repeatedly push status-only changes while seven PR checks are running. Existing seven CI checks are mandatory. Keep successful test/build logs outside agent context; fetch a failing step and 40–60 relevant lines only when actually needed.
3. Separate three expenses: AI/Copilot *credits/tokens*, GitHub Actions runner usage and GitHub Pro/Copilot subscriptions. The public repo's standard Actions runners are free under current GitHub terms. For real personal AI costs use [GitHub billing → AI usage](https://github.com/settings/billing), and configure hard-stop budgets/session limits **manually** where supported. Never invent a per-repo AI-token figure.
4. **Infra experiments:** P7 local smoke and P12 disposable rehearsal only with a clean test checkout and synthetic credentials; P14 static-only without actual-host approval. P8 PowerShell harness at main `1523d82f954` is **source-broken near lines 127–148**; no Docker workload until a separate parser-tested **INFRA-P8-01** fix. Record `PASS/FAIL/NOT RUN`, revision, host, OOM/cleanup and source without secrets.
5. Preserve security/native/FFI gates, approval before merge or deployment and real-host P14 release safeguards. Update only the small live queue after confirmed evidence. Do not let a speculative CI optimization or token-audit effort displace the next ready customer feature F2a browser-local Crop.

## Skill 21 — Product release milestones, P7/P8/P12/P14 evidence and post-reset Codex allowance

**Canonical guide:** [Product/infra milestone hierarchy and Codex usage experiment](docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md); [live board](next.md) and [Skill 18](skills.md) govern truthful statuses. Historical reports are in [next-history.md](next-history.md) and must not be preloaded. This is an agent *recipe*, not an executable skill or an account-usage API.

1. Read **only compact live next.md** + current GitHub main/PR/check metadata, then target scoped AGENTS and affected source; one bounded task at a time. Do not repeatedly fetch complete GitHub PR diffs or seven passing CI logs when local source/summary is sufficient. Preserve all required checks and exact-SHA review.
2. Track **parent product milestones** `PRODUCT-FREE-01`, `PRODUCT-PRO-01`, `PRODUCT-STUDIO-01`, `PRODUCT-API-01` separately from child PRs. **Free v1 has five mandatory tools** (Crop/Resize/Compress/Rotate/Convert): all five feature PRs through F4 Convert #76 are merged. A feature merge advances its child only; a parent passes only its full scope and independently approved release gates, and `RELEASED` means real user availability.
3. Track **infrastructure** `INFRA-P7` local dev, `INFRA-P8` VPS simulation, `INFRA-P12` disposable local candidate and `INFRA-P14` actual public-host readiness independently. Record source SHA, host, actual commands, result/exit, security and OOM/cleanup evidence per track. **P8 PowerShell source defect** must receive a separate tested repair before actual P8 workload; successful P8 simulation cannot validate P14 deployment.
4. Distinguish `PLANNED / READY / PR_OPEN / CI_PENDING / READY_FOR_REVIEW / MERGED / VERIFIED / READY_FOR_RELEASE_REVIEW / RELEASED` and explicitly `BLOCKED / NOT_RUN`. Never equate portfolio approval, committed docs, test PASS, merge and product shipment. After a merge, reconcile statuses in the next bounded authorized PR.
5. **After an actual allowance reset**, optionally compare two **read-only** VS Code Codex tasks on the same PR with same model/reasoning and no other concurrent agentic usage: local Git/file read only versus GitHub metadata/file read only. Record *privately* before/after **5-hour and weekly remaining percentage**, timestamps, reset crossings, tool calls, response sizes and UI update delay. If the reset or other usage contaminates trials, mark **INCONCLUSIVE**. Percent delta isn't a per-tool token invoice.
6. The user supplies screenshots/numbers voluntarily **in the private conversation**, not in the public GitHub repository. Never fetch/login to their ChatGPT account, store personal billing balances, infer real-time allowance, buy credits or enable automatic reload. Review model selection, context volume, repeated tool calls, prompt caching and local test output length; don't accuse GitHub Actions of charging Codex tokens.
7. For P7/P12 local experiments, synthetic keys and isolated Docker only; P14 no external actions without explicit deployment/spend/secret approval. No code/CI changes from a documentation milestone. Cross-functional review (engineering, security/SRE and finance/management) remains proportionate to risk.

## Skill 22 — Codex VS Code six-rule local-first and low-context execution

**Mandatory default method for future Astra/Codex tasks, subject to security/CI/owner rules:** [Six-rule Codex operating policy](docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md). Read the short policy when needed; do not preload all historical skills/docs/logs.

1. **Local workspace first:** inspect/edit/test from local checked-out source and Git; never use GitHub connector to re-download identical files/diffs already locally available without a specific source-of-truth reason.
2. **Minimal context:** short [live next.md](next.md), current task contract, relevant scoped AGENTS and affected sources; history/other product tracks on demand. Never omit binding safety instructions.
3. **Targeted remote GitHub:** PR creation, review status, exact SHA/required CI outcomes, merge confirmation; request status/changed filenames before patches, retrieve complete diff only with evidence-based need; no repeated pending polling or success logs.
4. **Compact testing:** focused local verification, full logs preserved outside agent context, PASS/FAIL/NOT RUN at exact SHA, failure excerpt first (40–60 relevant lines); all seven GitHub jobs and native release/FFI/security requirements stay intact.
5. **Risk-adjusted reasoning:** prefer standard/lower-cost model/effort *if actually available and suitable* for routine scoped tasks; escalate for native concurrency, C/Rust FFI, security, architecture or unresolved defects. No silent changes to user settings or guaranteed rate-card claims.
6. **Measure:** after 5-hour and weekly allowances reset, owner optionally shares private local-only vs GitHub-heavy matched read-only sessions, timestamps and usage % remaining. Account screenshots and payment details are **never** stored in public repository; see [experiment protocol](docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md).

**One-session report:** one READY task, local tests, exact GitHub PR/CI evidence, no invented results, no automatic merge or deploy; concise next action. Do not repeat work already merged in the four product parents or infra tracks.

## Skill 23 — F2c twelve-gate frontend CI and acceptance verification

**Use:** [F2c Compress quality matrix](docs/FRONTEND_F2C_CI_QUALITY_GATES.md); [scoped frontend AGENTS](imgengine-saas/frontend/AGENTS.md); [change-scoped verification](docs/CHANGE_SCOPED_VERIFICATION.md). This is a recipe, not an installed CI workflow or evidence that a test ran.

1. Inspect current main, the full local F2c dirty worktree (staged/unstaged/untracked) and exact feature candidate SHA. Do not overwrite local F2c changes; differentiate local claims from GitHub facts.
2. Run the pinned Node 24/Corepack/pnpm@10.34.9 locked frontend lane: `pnpm install --frozen-lockfile`, `pnpm exec nuxi prepare`, `pnpm test`, `pnpm run typecheck`, `pnpm run build`. If the toolchain is unavailable locally, use an approved CI/Linux runner rather than skipping checks.
3. Build coverage for all twelve matrix rows. Existing `pnpm run test:browser` only selects sheet-crop via the present Playwright config; first implement F2c test discovery and actual Chromium/Firefox/WebKit execution before reporting browser PASS. Include Worker cancellation, privacy/GPS metadata, dimensions, a11y, resource and regression cases.
4. Keep all seven existing GitHub required jobs intact. Implement/add a separately reviewed browser/12-gate CI layer; use named exact-SHA statuses, artifacts, and measured resource/performance evidence. No `continue-on-error`, `|| true`, unreviewed audit suppression, or fake test results.
5. Report every G01–G12 as PASS/FAIL/SKIPPED/NOT RUN/N/A with rationale, workflow URL, tested commit, run counts and blockers; update `next.md` only after status evidence warrants it. Request security/architecture/CI review as applicable, and do not merge/release merely because docs were updated.
