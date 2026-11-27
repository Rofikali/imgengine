# IMGENGINE approved technology policy and repository-structure ADR

**Owner-approved architecture direction — 27 November 2026:** One protected `main`, short-lived product-scoped feature branches, five product families with independently versioned, tested and released artifacts. Free remains the only active product; reuse visual UI without implementing paid logic. No permanent per-product branches, bulk directory moves, new toolchains, public paid source, new services or spending. This decision approves architecture direction and documentation, not releases or new product implementation. #130 merged `92703496208572f1ff07ec1cedb065bcf9b5b8af`, [main CI passed](https://github.com/Rofikali/imgengine/actions/runs/45976692568). #131 exact head `8818d6fa2258a3f4acfb066050535a7e008eb07d` has policy and frontend verification PASS, browser quality running at inspection ([run](https://github.com/Rofikali/imgengine/actions/runs/45977976019)). Preserve active CI; this documentation candidate is staged as an unreferenced commit for later integration, not pushed to the active branch or main. Candidate tests NOT RUN. See [approved portfolio direction](ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md).

Use current paths; no new C/Rust/Wasm dependency for Free, no bulk restructuring. Shared visual components only; enforce no-upload and independent product builds. Extract shared packages only after real reuse evidence.


**Decision:** ADR-TECH-001 — Accepted for **all new product development**, 2026-11-14.
**Status of implementation:** Target C17 + Rust/Axum + Nuxt already has implemented P5/P8/P12 slices; this ADR **does not** mean the legacy services have been removed or the target is publicly released.
**Audited repository baseline:** `fee24bdff441e6ef735afaa06c024d4948fb623f`. This document makes an explicit product-stack choice; it does not contain fresh CI, capacity or deployment evidence.

**Source audit:** [technology-policy consistency audit and existing Python file inventory](TECHNOLOGY_DOCUMENTATION_AUDIT_2026-11-14.md). This source audit is dated, is documentation-only, and does not claim any legacy code or Python verification scripts have been removed.

## 1. Non-negotiable technology direction

**The user's selected target is C17 for the native image-processing core, Rust for the backend and FFI, and the existing Nuxt/Vue/TypeScript frontend.** Do not propose or add a Python-based backend, queue, worker, data service, image engine, SDK integration, or new Python-based test/build tooling for this target.

| Layer | Approved target | Excluded for NEW work | Reality in current repository |
| --- | --- | --- | --- |
| Native image execution | Strict C17; C ABI; .so and, after packaging proof, public .a; TurboJPEG, measured SIMD | Python image processing, redundant Rust rewrite of working C scheduler | C core + public .so exist; public installable .a not established |
| Backend / HTTP | **Rust, Tokio, Axum**, bounded synchronous/ephemeral processing, versioned contracts, safe Rust FFI | **Python, FastAPI, Flask, Django, Django REST Framework (DRF)**; Python API frameworks or workers | Current Rust API is under `backend/imgengine-api/` |
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

## 3. Active source layout after merged PR #37

The repository moved existing Rust crates from `imgengine/rust/` to top-level `backend/` in **merged PR [#37](https://github.com/Rofikali/imgengine/pull/37)**. Seven GitHub CI jobs passed on exact PR head `4ab5a65fcde411fd77dde8eb36aea09dce7915c3`, merged as `e567616975476204c9279f476c28fb74e67512bc`. This verifies the structural integration and tested build at that revision, **not a public production release or a measured speed increase**. For new agent tasks, read [Astra engineering start guide](ASTRA_ENGINEERING_START_HERE.md).

```text
imgengine/                        # C17 native image-processing engine
  AGENTS.md                       # native safety + C ABI rules
  include/imgengine/api/          # public C ABI
  src/                            # native decoder, layout, scheduler, encoder
backend/                          # ACTIVE RUST BACKEND on main
  AGENTS.md                       # Rust/Axum/FFI safety and architecture
  Dockerfile                      # root-context native C + Rust API build
  docker-entrypoint.sh
  imgengine/                      # safe Rust wrapper over native C ABI
  imgengine-supervisor/           # bounded admission/request lifecycle
  imgengine-api/                  # Axum HTTP product API
  imgengine-ffi-smoke/            # external native FFI smoke
imgengine-saas/
  frontend/
    AGENTS.md                     # active Nuxt 4/Vue/TS presentation
    server/api/                   # typed same-origin Rust proxies
  backend/
    AGENTS.md                     # RETAINED LEGACY PYTHON ONLY
    api-service/                  # old FastAPI/PostgreSQL job service
    worker-service/               # old Celery workers
infra/                             # build/deploy candidate; paths updated in refactor
docs/                              # contracts, architecture, evidence
AGENTS.md                           # repository policy
skills.md                           # optional workflow recipes
next.md                             # task status register
```

**Agent scope:** root AGENTS governs all; `imgengine/AGENTS.md` governs **native C**; `backend/AGENTS.md` governs **Rust/Axum, supervisor and FFI**; `imgengine-saas/frontend/AGENTS.md` governs Nuxt; `imgengine-saas/backend/AGENTS.md` governs **only legacy Python**. This prevents the misleading old folder name from routing Rust work into Python.

## 4. Migration and safe folder policy

- **Stage S0 — DONE (merged PR #37):** relocated the existing Rust crates to `backend/`, adjusted Docker/Cargo/CI/verification paths and passed all seven required CI jobs at the PR head. This was a structural change, not a new feature, kernel rewrite, production release or authorization to add Python/Redis/PostgreSQL.
- **Stage S1 — optional subsequent cleanup:** do **not** move `imgengine-saas/backend/` automatically. It contains still-retained Python source and migrations; a future separate PR can archive or rename only after legacy-data, API parity, backup, rollback, container, CI and deployment implications are reviewed.
- **Stage S2 — approved legacy retirement:** only after route/behavior and data-disposition parity, canary/recovery tests, rollback window and explicit human approval. Renaming a folder is not retirement.
- **Stage S3 — paid product modules:** Rust-only application modules when needed; a separate approved durable-metadata decision, no automatic Redis/PostgreSQL/queue introduction.

**No premature cleanup:** Do not remove old Python source or critical Python verification scripts in future refactors; they still serve current legacy or native/release gates. Don't rewrite them without equivalent safe test evidence. The top-level `backend/` move is an organizational choice, with **no presumed image-processing speed improvement**.

## 5. Architecture and verification rules

- Layering: browser → Nuxt server proxy → **Rust Axum HTTP adapter** → lifecycle/admission → **safe Rust FFI** → versioned C ABI → C scheduler → C17 decoder/layout/encoder. Keep ownership, stable error schemas, bounded memory/CPU/workspaces, correct cancellation semantics and redaction at the correct boundary.
- For native and FFI changes: apply C17/C11 public-ABI and Rust compatibility tests, relevant security/sanitizer/fuzz gates and actual release verification. Keep safe ownership and no unproved `Send`/`Sync` assumptions.
- For isolated Nuxt UI, worker/Wasm prototypes or Rust-only non-FFI code: focused tests/typecheck/build and relevant browser/Rust tests; **do not manually rebuild native C or rerun ASan/UBSan/fuzz** unless a native/shared boundary changes. This is a local developer-loop efficiency policy; existing required GitHub CI/release checks are not disabled.
- For legacy changes solely needed to protect migration/rollback: make narrowly scoped security fixes with appropriate legacy tests; do not add new Python features or dependencies to the target product.
- For docs-only changes: status accuracy, relative links, and diff scope; no manual C/Rust/frontend rebuild.
- Performance claims: nanoseconds may be relevant for tiny primitives, microseconds for small kernels; full decode/render/encode must be **measured** and may take milliseconds or longer. No safety bypass for speed.

## 6. Authority, cross-linking and change control

1. [Root AGENTS](../AGENTS.md) — global engineering constraints, including this policy.
2. [Native AGENTS](../imgengine/AGENTS.md) and [Rust AGENTS](../backend/AGENTS.md) — C and Rust/FFI scopes.
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
| Rust crates moved into top-level backend/ | **DONE / MERGED**, 7/7 CI at PR #37 head | Preserve paths and exact-version verification in subsequent changes |\n| Moving/retiring old Python backend | **DEFERRED** | Separate approved migration/rollback/data-retention gate |
