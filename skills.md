# IMGENGINE — Engineering Skills and Workflow Recipes

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
