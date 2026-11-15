# IMGENGINE Documentation

This directory contains current contracts, dated evidence, proposed designs, and legacy/historical records. A requirement is binding only within its named product and versioned scope: current public ABI/API source and approved contracts take precedence over historical PRDs or RFCs. Use the [Native C/Rust Documentation Map](NATIVE_C_RUST_DOCS_MAP.md) for the source-of-truth hierarchy.

The root [next.md](../next.md) is the execution/status dashboard. The optional [skills.md](../skills.md) supplies reusable task recipes; root and nested AGENTS.md files remain authoritative instructions.

For concrete C17/Rust subsystem ownership, HLD/LLD contracts, security and observability boundaries, DSA/patterns, open risks and measurement requirements, use the [Native/Rust Contract-to-Evidence Matrix](NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md). The [81-file map](NATIVE_C_RUST_DOCS_MAP.md) and [dated audit](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) preserve their *original snapshot*; the audited pre-Astra `main` snapshot had **89 Markdown files under `docs/`**; merged PR #38 added Astra's 90th document, merged PR #39 added the 91st infra audit, and this C17 source-audit branch adds the 92nd. Treat these as **dated counts**, not an ever-current total. Those historical counts must not be reused as an up-to-date total.

For Nuxt/Vue/TypeScript and the retained FastAPI/Celery backend, use the [Frontend and Legacy Backend Audit](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md). This is the source-aligned state/authority guide for the two scoped [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md) and [legacy backend AGENTS](../imgengine-saas/backend/AGENTS.md), not a production release certificate.

The root [living next-action dashboard](../next.md) now includes cross-workstream priorities, exact scope ownership for frontend/legacy Python/native Rust and status update rules. The [original 2026-08-17 `next.md` backlog](HISTORICAL_NEXT_2026-08-17.md) is preserved verbatim from Git history and **is not** an active requirement. Agent files define policy, not milestone status; the [optional skills recipes](../skills.md) define workflows.

**Approved future application stack:** C17 native core + safe Rust FFI + Rust/Axum/Tokio backend + current Nuxt/Vue/TypeScript frontend. **No new Python/FastAPI, Redis, PostgreSQL, Celery, or Python tooling.** Existing legacy Python sources and historical docs remain only for controlled maintenance/migration, not as new-work guidance. See [binding technology and repository-structure ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md) and [Rust AGENTS](../backend/AGENTS.md); do not move/delete the legacy folder as part of a documentation change.

For the **source-backed policy reconciliation and exact inventory of existing Python-related files**, consult [Technology Documentation Audit (2026-11-14)](TECHNOLOGY_DOCUMENTATION_AUDIT_2026-11-14.md). It distinguishes a binding no-new-Python/Rust-only decision from the retained legacy Python code and verification scripts.

**Rust backend relocation complete (PR #37):** [Rust Backend Directory Migration](RUST_BACKEND_DIRECTORY_MIGRATION.md) records the move of Rust/Axum, FFI and supervisor crates to top-level `backend/`, verified by [7 passing GitHub CI jobs](https://github.com/Rofikali/imgengine/actions/runs/43618598771). Native C17 remains in `imgengine/`; Python in `imgengine-saas/backend/` remains legacy. This is **not** a public production release.

**Infrastructure entry point:** [Infra Security/Operations/Economics Audit (2026-11-15)](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [infra AGENTS](../infra/AGENTS.md) and [infra README](../infra/README.md) cover the 18 tracked active infrastructure files: P7 local dev, P8 simulation, P12 disposable candidate and P14 production configuration. Read with [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md). Static Docker policy checks and local image builds are **not** public VPS security or capacity proof.

**Latest C17 deep source audit:** [C17 Core Principal Source, LLD, Design Patterns, DSA and Security Audit (2026-11-15)](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) and [native C core README](../imgengine/README.md). The report traces actual ABI/allocator/queue/worker/codec/resize code and ranks new concurrency, resource, sandbox, portability and SDK findings without claiming tests were rerun. Historical HLD/LLD and pattern suggestions are not current source truth.

**Seven-level engineering governance:** [Seven-Level Engineering Operating Model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) gives an agent-neutral Junior-to-Distinguished technical review and security/SRE/CA/MBA management framework; [Agent Milestone Handoff Template](AGENT_MILESTONE_HANDOFF_TEMPLATE.md) gives bounded inputs, acceptance/evidence requirements and copy-paste Astra/Codex instructions. These are documentation-only governance artifacts, not feature completion or production evidence.

**Product/business/performance strategy proposal (2026-11-15):** [Platform Product & Business Strategy](IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md) covers free-versus-paid, Studio workflow, later API/Desktop/SDK channels, customer discovery, finance/CA-MBA and market hypotheses. [C17/Rust/Wasm/API/Desktop Performance Plan](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md) defines workload-specific measurements, C concurrency/RSS safeguards, target-specific code architecture and release gates. [STRAT-00 Agent First Move](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md) supplies a read-only Astra/Codex/human-engineer brief. **No pricing, new priorities, implementation milestones or production statuses were automatically approved by these docs.**

## Start Here

**For an AI coding/review agent such as Astra, start with [Astra engineering start guide](ASTRA_ENGINEERING_START_HERE.md).** It defines source-backed scope, three bounded C17/Rust/Nuxt tracks, P13 sequencing, security and performance gates, and business review criteria.

| Document | Purpose | Audience |
| --- | --- | --- |
| [Infrastructure Audit (2026-11-15)](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) | Active P7/P8/P12/P14 Docker/Caddy source inventory, static/live control boundaries, actual-VPS blockers, secrets, supply chain, operations and CA/MBA economics. | Infra, security, SRE, release, Astra reviewers |
| [Frontend and Legacy Backend Audit (2026-11-14)](FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md) | Implemented Nuxt P5/P8/P12, incomplete crop/production, retained FastAPI/Celery, security/operations gaps and agent scopes. | Frontend, legacy/backend, security, product owners |
| [Native C/Rust Documentation Audit (2026-11-14)](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) | Source-backed security, observability, HLD/LLD, FFI, performance and documentation conflicts with status/evidence. | Native C and Rust reviewers |
| [Native C/Rust Documentation Map](NATIVE_C_RUST_DOCS_MAP.md) | Original dated 81-document classification; historical snapshot rather than the complete current index. | All contributors |
| [Project Status & Decision Register](PROJECT_STATUS_AND_DECISION_REGISTER.md) | Living inventory of implemented, partial, deferred, legacy and blocked work, with evidence and decisions. | All contributors |
| [Browser-First Product Roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) | Proposed image-tool routes, Rust/Wasm milestones, free/paid features, and business gates. | Product, architecture, engineering |
| [Change-Scoped Verification](CHANGE_SCOPED_VERIFICATION.md) | Fast developer-loop testing and AI token/time budget policy without weakening release gates. | All contributors, CI owners |
| [PRD (legacy asynchronous product)](PRD.md) | Historical durable-job SaaS scope and requirements; the current Rust MVP uses api.md and MVP_RELEASE_CHECKLIST.md. | Product, engineering, operations |
| [Engine Specification](ENGINE_SPEC.md) | Native C/assembly layout contract: size, border, crop, bleed, output, and performance boundaries. | Native engineers, bindings authors |
| [Rust API Contract](api.md) | Implemented synchronous Rust/Axum render, layout, and physical-sheet contracts. | Frontend, backend, and platform engineers |
| [Local Docker Rehearsal](LOCAL_DOCKER_REHEARSAL.md) | Local P12 candidate rehearsal and its explicit evidence limits. | Developers and release owners |
| [SaaS Contract (legacy)](SAAS_CONTRACT.md) | Historical FastAPI, queue, worker, storage, and durable-job contract. | Legacy-path maintainers |
| [Engineering Playbook](ENGINEERING_PLAYBOOK.md) | How a loop moves from a requirement to verified production evidence. | All contributors |
| [SRE Contract (legacy durable jobs)](SRE.md) | Retained FastAPI/Celery job/queue objectives and incident rules; not Rust MVP SLO evidence. | Platform and on-call engineers |
| [Security Operations (legacy controls)](SECURITY_OPERATIONS.md) | Legacy durable-job API-key/job-ownership controls and future tenancy gates; not implemented Rust paid tenancy. | Security and platform engineers |
| [Release Checklist (legacy durable-job)](RELEASE_CHECKLIST.md) | Historical asynchronous release governance; Rust physical-sheet release uses MVP_RELEASE_CHECKLIST.md. | Release owners |
| [Database Recovery (legacy)](DATABASE_RECOVERY.md) | Retained PostgreSQL durable-job backup/restore; not a dependency of the Rust ephemeral MVP. | Platform and on-call engineers |
| [Native Development](NATIVE_DEVELOPMENT.md) | Supported Linux toolchain and reproducible Loop A commands. | Native engineers, CI owners |
| [Native Benchmarking](NATIVE_BENCHMARKING.md) | Reproducible native performance measurement contract and evidence policy. | Native and performance engineers |
| [Linux Native Verification](LINUX_VERIFICATION.md) | Ubuntu 24.04 Docker verification gate for C correctness and security. | Native engineers and CI owners |
| [Rust FFI Boundary](RUST_FFI.md) | Safe ownership boundary over `libimgengine` ABI v1. | Rust and native engineers |
| [Rust Supervisor Foundation](RUST_SUPERVISOR.md) | Ephemeral single-engine request lifecycle proof. | Rust and platform engineers |
| [Rust Request Lifecycle](RUST_REQUEST_LIFECYCLE.md) | Transport-neutral request ownership, cleanup, timeout, and error contract. | Rust and platform engineers |
| [Scheduler Benchmark Contract](SCHEDULER_BENCHMARK_CONTRACT.md) | Decision gate for C execution and Rust admission evidence. | Rust and native engineers |
| [Scheduler Characterization Report](SCHEDULER_CHARACTERIZATION_REPORT.md) | Recorded decision gate result: keep C scheduler ownership. | Architecture reviewers |
| [HLD](hld.md) | Native architecture and performance constraints. | Native engineers |
| [LLD](lld.md) | Native module mappings, ownership, and call flows. | Native engineers |
| [Design Patterns](design_patterns.md) | Pattern guidance for the native implementation. | Native engineers |
| [v2 RFC](v2.0%20RFC.md) | Historical high-performance architecture proposal. | Architecture reference |
| [Architecture](architecture.md) | Implemented Rust/C/Nuxt first-MVP boundaries, retained legacy path, and open release gates. | Architecture and platform engineers |
| [Migration Plan](migration.md) | Reversible legacy-to-Rust migration position and retirement criteria. | Engineering and release owners |
| [Security Design](security.md) | Target security controls and migration gates. | Security and platform engineers |
| [Threat Model](threat-model.md) | Assets, threats, mitigations, and verification. | Security and engineering |
| [Deployment Design](deployment.md) | Hardened low-cost VPS operating model. | Platform and on-call engineers |
| [Performance Strategy](performance.md) | Native and end-to-end measurement gates. | Native and performance engineers |

## Documentation Rules

1. Mark each statement as **implemented**, **planned**, or **validated** when that distinction matters.
2. Define public behavior in a contract before implementing it in C, Rust/Axum, Nuxt, FastAPI, or a worker.
3. Update the matching contract and acceptance criteria in the same change as an externally visible behavior change.
4. Do not advertise benchmark targets until CI produces a reproducible benchmark result.
5. Prefer stable names and versioned payloads over undocumented path or process assumptions.

## Product Vocabulary

- **Photo:** a source JPEG or PNG.
- **Cell:** one positioned, scaled copy of the source photo on a sheet.
- **Sheet/canvas:** the final printable output containing one or more cells.
- **Layout job:** dimensions, grid, print-safety, color, and output parameters used to render a sheet.
- **Job:** a durable SaaS record in the legacy asynchronous architecture; the target synchronous architecture has no durable job artifact.
- **Control plane:** implemented Rust/Axum validation, authorization, lifecycle, and bounded admission; FastAPI remains the retained legacy control plane.
- **Execution plane:** retained C scheduler and native engine process pixels; legacy workers remain only on the retained legacy path.

## Status Legend

- **Implemented:** present in the tracked source; may still need tests.
- **Validated:** built or tested with recorded evidence.
- **Planned:** accepted requirement not yet implemented.
