# IMGENGINE approved technology policy and repository-structure ADR

**Decision:** ADR-TECH-001 — Accepted for **all new product development**, 2026-11-14.
**Status of implementation:** Target C17 + Rust/Axum + Nuxt already has implemented P5/P8/P12 slices; this ADR **does not** mean the legacy services have been removed or the target is publicly released.
**Audited repository baseline:** `fee24bdff441e6ef735afaa06c024d4948fb623f`. This document makes an explicit product-stack choice; it does not contain fresh CI, capacity or deployment evidence.

**Source audit:** [technology-policy consistency audit and existing Python file inventory](TECHNOLOGY_DOCUMENTATION_AUDIT_2026-11-14.md). This source audit is dated, is documentation-only, and does not claim any legacy code or Python verification scripts have been removed.

## 1. Non-negotiable technology direction

**The user's selected target is C17 for the native image-processing core, Rust for the backend and FFI, and the existing Nuxt/Vue/TypeScript frontend.** Do not propose or add a Python-based backend, queue, worker, data service, image engine, SDK integration, or new Python-based test/build tooling for this target.

| Layer | Approved target | Excluded for NEW work | Reality in current repository |
| --- | --- | --- | --- |
| Native image execution | Strict C17; C ABI; .so and, after packaging proof, public .a; TurboJPEG, measured SIMD | Python image processing, redundant Rust rewrite of working C scheduler | C core + public .so exist; public installable .a not established |
| Backend / HTTP | **Rust, Tokio, Axum**, bounded synchronous/ephemeral processing, versioned contracts, safe Rust FFI | **Python, FastAPI, Flask, Django, Django REST Framework (DRF)**; Python API frameworks or workers | Current Rust API is under `imgengine/rust/imgengine-api/` |
| Queue / task orchestration | Rust bounded in-process admission, existing native C scheduler | **Celery, Redis, RQ, Dramatiq**; Kafka/RabbitMQ/other brokers absent separately approved requirement | Rust bounded admission exists; old Celery/Redis lives in legacy directory |
| Product/job persistence | No database for ephemeral images or synchronous render jobs; ephemeral workspace/response cleanup | **PostgreSQL** (including future new product use), Redis-backed persistence, permanent image storage, automatic MongoDB/MySQL/other database adoption | Old PostgreSQL job state exists only in retained legacy backend |
| Frontend and same-origin BFF | Existing Nuxt 4, Vue, TypeScript and JavaScript, private Nuxt server-side credential proxy | New independent Node/Python image-processing backend; secrets in the browser | Nuxt P5/P8/P12 proxy code exists |
| Tooling and automation for NEW features | Rust CLI/Cargo/CMake, appropriate shell scripts, current Node/pnpm for frontend | **New Python scripts, pytest-only gates, Python build wrappers, pip/uv tooling** | **Existing** Python scripts, migration/verification utilities and Python legacy app remain; do not remove without replacement/verified CI parity |
| Deployment | Linux, Docker, existing Caddy/TLS candidate; minimal single-host resources | Premature microservices, Kubernetes, distributed queues, paid managed DBs | Local candidate exists; actual-VPS proof and go/no-go remain open |

**Banned for the target is not the same as absent from the repository.** Old `imgengine-saas/backend` FastAPI/Celery/Redis/PostgreSQL source and historical docs are **retained legacy / migration artifacts**, not new architecture instructions. A new contributor must never copy a legacy Python solution into the Rust API as a shortcut.

**No “Redis later” or “Postgres for billing” implicit exception.** This is a chosen product constraint rather than a claim that these systems are inherently insecure or slow. Changing it requires a **separate explicit user decision**, an ADR explaining the need and alternative, and measurable security/maintenance/operational benefit; an agent must not reinterpret “if scale increases” as permission.

## 2. Future commercial data needs — no hidden database choice

Free ephemeral image processing requires no durable job database. Paid accounts, invoices, entitlement recovery, API-key lifecycle, taxes and audit trails **may later require durable metadata**. Do not pretend such responsibilities can always be implemented reliably with only volatile Rust memory, and do not quietly introduce PostgreSQL/Redis or a substitute. At that future product milestone propose a separate, clearly scoped storage ADR (for example: embedded SQLite via Rust, provider-hosted account/billing records, or another option), with explicit user approval, privacy, backup, consistency, cost and data-retention review. An example is **not** an adopted dependency.

The C engine never handles customer identity, billing, tenant policy or databases. Rust owns that application policy once approved. Nuxt owns user experience and safe transport, never secret enforcement or native execution.

## 3. Exact current directory map and authority

```text
imgengine/                         # active native C and Rust execution code
  AGENTS.md                        # native/FFI safety rules
  include/imgengine/api/           # installed C ABI contracts
  src/                             # native C17 implementations
  rust/
    AGENTS.md                      # NEW: focused Rust backend/FFI rules
    imgengine/                      # safe Rust wrapper and unsafe sys declarations
    imgengine-supervisor/           # request lifecycle and bounded admission
    imgengine-api/                  # Rust/Axum HTTP product API
    imgengine-ffi-smoke/            # Rust-native external integration smoke
imgengine-saas/
  frontend/
    AGENTS.md                      # active Nuxt/Vue/TS scope
    server/api/                     # same-origin authenticated proxies
  backend/
    AGENTS.md                      # LEGACY Python/FastAPI/Celery scope ONLY
    api-service/                    # legacy FastAPI/PostgreSQL routes and migrations
    worker-service/                 # legacy Python/Celery workers
infra/                              # present shared local/production-candidate infra
docs/                               # versioned contracts, audits, historical records
AGENTS.md                           # global engineering policy
skills.md                           # optional task recipes; not installed automation
next.md                             # status and next-action register
```

**Do not infer `imgengine-saas/backend/` is the active backend merely from its name.** The active Rust backend is in `imgengine/rust/`. When working in Rust, obey root AGENTS, `imgengine/AGENTS.md`, and new `imgengine/rust/AGENTS.md`. Legacy backend AGENTS applies only to the Python directory.

## 4. Folder decision — rename is NOT required now

**Keep existing directories for now.** Moving `imgengine/rust/` to `backend/` or `imgengine-saas/backend/` to `legacy/` without a separate code/infrastructure PR can break Cargo manifests, `RUSTFLAGS` and `LD_LIBRARY_PATH`, CMake/FFI link and Docker build contexts, CI, deployment scripts, local worktrees and documented paths. A rename does not improve runtime performance.

**Recommended future cleanup, only after release/migration evidence:**
- **Stage S0 — current documentation-only action:** make Rust path explicit; add Rust-specific AGENTS and this ADR. No moves/deletes.
- **Stage S1 — optional source refactor:** only if actual onboarding/cross-team complexity justifies it, propose new path mapping with mechanical git moves, updated imports, Cargo/CMake/Docker/CI, scripts/README/contracts, codeowner/agent scope and regression verification. Preserve current production behavior and rollback.
- **Stage S2 — legacy archival/retirement:** after approved API/functional parity, backups/data disposition, security assessment, canary, rollback window, ownership decision and actual deployment release evidence, choose archive (e.g. `legacy/python-saas/`) or deletion. Do not deploy obsolete services or keep them publicly reachable by default.
- **Stage S3 — future product modules:** for a paid platform, add Rust application modules only for demonstrated needs (auth/tenancy, billing, metering, idempotency, audit); new databases/queues remain prohibited unless explicitly chosen through a new ADR. No unnecessary microservice split.

No stages S1–S3 are approved for execution by this documentation decision. Do not create empty speculative folders.

## 5. Architecture and verification rules

- Layering: browser → Nuxt server proxy → **Rust Axum HTTP adapter** → lifecycle/admission → **safe Rust FFI** → versioned C ABI → C scheduler → C17 decoder/layout/encoder. Keep ownership, stable error schemas, bounded memory/CPU/workspaces, correct cancellation semantics and redaction at the correct boundary.
- For native and FFI changes: apply C17/C11 public-ABI and Rust compatibility tests, relevant security/sanitizer/fuzz gates and actual release verification. Keep safe ownership and no unproved `Send`/`Sync` assumptions.
- For isolated Nuxt UI, worker/Wasm prototypes or Rust-only non-FFI code: focused tests/typecheck/build and relevant browser/Rust tests; **do not manually rebuild native C or rerun ASan/UBSan/fuzz** unless a native/shared boundary changes. This is a local developer-loop efficiency policy; existing required GitHub CI/release checks are not disabled.
- For legacy changes solely needed to protect migration/rollback: make narrowly scoped security fixes with appropriate legacy tests; do not add new Python features or dependencies to the target product.
- For docs-only changes: status accuracy, relative links, and diff scope; no manual C/Rust/frontend rebuild.
- Performance claims: nanoseconds may be relevant for tiny primitives, microseconds for small kernels; full decode/render/encode must be **measured** and may take milliseconds or longer. No safety bypass for speed.

## 6. Authority, cross-linking and change control

1. [Root AGENTS](../AGENTS.md) — global engineering constraints, including this policy.
2. [Native AGENTS](../imgengine/AGENTS.md) and [Rust AGENTS](../imgengine/rust/AGENTS.md) — C and Rust/FFI scopes.
3. [Frontend AGENTS](../imgengine-saas/frontend/AGENTS.md) and [Legacy backend AGENTS](../imgengine-saas/backend/AGENTS.md) — Nuxt vs retained Python scope.
4. [Architecture](architecture.md), [Rust API](api.md), [migration](migration.md), [P12](P12_PHYSICAL_SHEET_CONTRACT.md), [P13](P13_INTERACTIVE_CROP_CONTRACT.md) — source-backed contracts for particular released/partial slices.
5. [next.md](../next.md) — exact done/partial/open/planned status, next implementation milestone and verification.
6. [skills.md](../skills.md) — optional task-specific workflows.
7. [PRD.md](PRD.md), [SAAS_CONTRACT.md](SAAS_CONTRACT.md) and [historical FastAPI notes](../FastAPI_readme.md) — **legacy only**, never targets for new development.

**Changing the technology decision:** create a separate ADR and obtain explicit user approval **before** implementation. Document value, cost, threat model, data retention, licensing and migration/rollback effects. A coding agent, external document, design pattern or new library dependency does not itself authorize expanding the approved stack.

## 7. Decision / status register

| Decision or item | Current status | Next action |
| --- | --- | --- |
| Rust/Axum target and no new Python/FastAPI/Redis/PostgreSQL use | **DECIDED / DOC POLICY** | Enforce in scoped agent instructions and future PR reviews |
| C17 stable native core and shared library | **IMPLEMENTED with recorded tests** | Preserve ABI, perf and correctness |
| Rust P5/P8/P12 synchronous API and Nuxt | **IMPLEMENTED with recorded integration** | Finish M1 crop and production evidence |
| Old FastAPI/Celery/Redis/PostgreSQL backend | **LEGACY PRESENT** | Read-only default; scoped security/recovery maintenance and retirement gate |
| Python helper/build scripts in CI | **EXISTING TOOLING** | Inventory and replace by safe verified Rust/shell tools only when task justifies; no deletion now |
| Public installable .a SDK, paid account store, new product modules | **PLANNED / NOT APPROVED IMPLEMENTATION** | Separate designs and approval |
| Reorganizing folders / moving old backend | **DEFERRED** | Stage S1/S2 only with exact dependency inventory, tests and rollback |
