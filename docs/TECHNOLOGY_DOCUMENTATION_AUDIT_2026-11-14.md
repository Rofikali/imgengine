# IMGENGINE technology-policy consistency audit — 2026-11-14

**Basis:** tracked `main` at `fee24bdff441e6ef735afaa06c024d4948fb623f`. **Audit type:** static GitHub source/document inventory; this PR changes Markdown policy only, with no compiled code, runtime settings, CI or migration. Findings are limited to reviewed repository content and are not a full dependency-license or runtime verification.

> **Subsequent source-layout update (draft refactor):** The original audit's `imgengine/rust/` source paths describe the **2026-11-14 baseline** and remain correct at that historical revision. A separate branch moves those Rust crates to top-level `backend/` with updated Cargo/CI/Docker paths. Until required checks pass and that PR merges, this is **not a verified change on main**. See [living technology decision](TECHNOLOGY_AND_REPOSITORY_POLICY.md).

## Decision

The user explicitly selects **C17 native IMGENGINE core → safe Rust FFI → Rust/Tokio/Axum HTTP/application backend → Nuxt/Vue/TypeScript frontend**. For **all new target product work**, do **not** use Python, FastAPI, Flask, Django/DRF, Celery, Redis, PostgreSQL, new Python scripts/tooling, or a newly introduced substitute queue/database without an explicit separately approved ADR. Rust/C and Nuxt are **existing**, not projects to rewrite. Refer to the [binding technology/structure ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md).

## Audit: conflicting or unclear documents

| Scope/file | What the baseline said | Correction / classification | Remaining action |
| --- | --- | --- | --- |
| [Root AGENTS](../AGENTS.md) | Rust/Axum target, but banned only “PostgreSQL for job-only persistence” | Explicitly excluded PostgreSQL and other old stack tech for **any new target** feature, not only job DB | Keep legacy/verification tools unchanged until evidence-backed retirement |
| [Native AGENTS](../imgengine/AGENTS.md) | C scheduler/FFI rules; cautioned against creating a full Rust backend during ABI milestone | Cross-link dedicated [Rust AGENTS](../backend/AGENTS.md): no ambiguity between ABI-only scope and the existing Rust/Axum service | Update status with real tests at later implementation milestones |
| [Legacy backend AGENTS](../imgengine-saas/backend/AGENTS.md) | Correctly notes Python legacy, but said Redis “may be reconsidered later” and limited PostgreSQL prohibition to jobs | Remove default future-Redis option; explicitly exclude PostgreSQL for new product; keep legacy recovery policy | Review legacy vulnerabilities and migration separately, not here |
| [Frontend AGENTS](../imgengine-saas/frontend/AGENTS.md) | Accurate P5/P8/P12 Nuxt→Rust proxies, but stack prohibition implicit | Explicit no new Python API, Redis/Postgres and link Rust owner | P13 crop UI/transport still partial |
| [Migration](migration.md) | Said “Redis/PostgreSQL remove from MVP” | Make technology choice permanent for new development unless user approves a new ADR | Existing legacy retirement blocked by parity/canary/rollback |
| [Architecture](architecture.md) | Had "no premature infrastructure" and Rust target, but not an explicit future ban | Clarified chosen stack vs retained legacy | Actual VPS/deployment gates open |
| [PRD](PRD.md) | Historical legacy disclaimer yet status still “Living document”; describes FastAPI/SQL/Celery as product architecture | Mark as historical legacy-only PRD and point at active Rust contracts/ADR | Future paid-product requirements need new approved product PRD |
| [Engineering Playbook](ENGINEERING_PLAYBOOK.md) | Contains Pydantic, Redis/Postgres worker steps even with legacy disclaimer | Mark entire Python SaaS checklist as legacy-specific; new Rust loop is distinct | Keep old evidence for legacy service only |
| [Roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) | Said no Python runtime in target while leaving tooling / future tiers implicit | Explicitly ban new Python scripts and old databases/queues for target; retain old tooling until replaced | Browser Worker/Wasm remains planned |
| [README](../README.md), [docs index](README.md), [next](../next.md), [skills](../skills.md), [contributing](../CONTRIBUTING.md), [project register](PROJECT_STATUS_AND_DECISION_REGISTER.md) | Mixed status, references to old architecture and live plans | Cross-link binding stack ADR, scoped Rust agent, folder/legacy status, and testing rules | Keep statuses synchronized after actual implementation |
| [Legacy SaaS contract](SAAS_CONTRACT.md), [SRE](SRE.md), [OBSERVABILITY](OBSERVABILITY.md), [Database Recovery](DATABASE_RECOVERY.md), [FastAPI notes](../FastAPI_readme.md) | Document historic Python services | **Retain, marked LEGACY/HISTORICAL**. Not new backend guidance | Do not delete until migration/data/rollback gates |
| [Native ABI](ABI.md), [P12](P12_PHYSICAL_SHEET_CONTRACT.md), [P13](P13_INTERACTIVE_CROP_CONTRACT.md), [Rust API](api.md), [Rust FFI](RUST_FFI.md) | Current implemented/partial technical contracts | **No functional technology conflict found warranting rewrite for this ADR** | Continue normal versioned contract rules |

## Python-related tracked file inventory and technical risk

The baseline Git tree contains **46 Python-related tracked files**, counting `.py` plus `pyproject.toml`, `uv.lock`, and `.python-version`: **32** within `imgengine-saas/backend/`, **14** outside. This is **not** 46 executable Python scripts. These counts are exact for the audited Git tree, not for untracked local files or transitive dependencies.

**Legacy backend (32):** `imgengine-saas/backend/api-service/` FastAPI/PostgreSQL/SQL migrations, `backend/worker-service/` Celery/Python worker and their Python/tooling manifests. Existing code is **LEGACY PRESENT**, not part of the approved new backend.

**Existing Python-related scripts outside legacy (14):**

```text
imgengine-saas/check_dependencies.py
imgengine-saas/scripts/check_compose_loopback_bindings.py
imgengine-saas/scripts/create_image_manifest.py
imgengine-saas/scripts/test_check_compose_loopback_bindings.py
imgengine/scripts/add_file_path_on_top.py
imgengine/scripts/analyze_benchmark_baseline.py
imgengine/scripts/bench_mark.py
imgengine/scripts/check_exported_symbols.py
imgengine/scripts/check_headers.py
imgengine/scripts/clang-format-check.py
imgengine/scripts/create_release_manifest.py
imgengine/scripts/create_runtime_sbom.py
imgengine/scripts/l10plusplus.py
infra/scripts/verify-production-compose.py
```

Some of these participate in the **native ABI, release/SBOM, Compose policy or CI safety gates**. Do not remove them merely to obtain a zero-Python file count. Replace each only in a separate, explicit **Rust or carefully constrained shell tool** milestone that proves input/output, failure-mode and CI parity and keeps release/security gates mandatory. No new Python tooling in the meantime. Deleting all legacy files is **not the same as completing migration**.

## Folder structure: recommendation vs implementation

**Current implemented:**
- `imgengine/`: core C17 and active Rust implementation under `imgengine/rust/` (now with dedicated Rust AGENTS).
- `imgengine-saas/frontend/`: implemented Nuxt 4.
- `imgengine-saas/backend/`: retained Python/FastAPI/Celery (misleading generic directory name).
- `imgengine-saas/infra/`: old Compose, database and observability topology; `infra/`: current Rust/Nuxt/Caddy candidate configs.

**Recommendation:** **do not move or delete directories now**. Renaming may affect Docker contexts, CI, Cargo manifests, CMake link paths, scripts, runbooks and local dirty worktrees. A future separate, manually approved PR can (a) inventory all references and data, (b) move or archive the old Python backend to a visibly legacy path if warranted, (c) preserve Git/history/rollback evidence and (d) run relevant integration gates. It may instead keep the current path permanently if a rename has no measurable benefit. **Moving the active Rust source is not needed** simply because it sits under the `imgengine/` tree.

## Handoff ledger

| ID | Area | Status | Exit condition |
| --- | --- | --- | --- |
| TECH-01 | Rust-only new backend policy | **DECIDED / DOCUMENTED** | New backend PRs follow Rust AGENTS, no banned additions |
| TECH-02 | Core C17 + Rust/Axum + Nuxt P5/P8/P12 | **IMPLEMENTED / prior recorded test evidence** | P13 M1 and release gates remain separate |
| TECH-03 | Python FastAPI/Redis/PostgreSQL service | **LEGACY RETAINED**, not new product | Parity, data handling, canary, rollback and explicit retirement |
| TECH-04 | Old Python verification scripts | **EXISTING**, replacement not implemented | Per-tool Rust/shell parity and updated required CI |
| TECH-05 | Physical folder move | **DEFERRED**, no move in docs PR | Proven need, path inventory, behavior-preserving PR, tests and rollback |
| TECH-06 | Future paid customer metadata storage | **NOT CHOSEN** | User-approved data/privacy/backup/cost ADR; do not silently add PostgreSQL/Redis |
| TECH-07 | Full production release | **BLOCKED** | Actual VPS, TLS/secrets, resource budget, monitoring, canary, rollback and explicit go/no-go |

**Assurance limit:** Documentation here defines *policy*; it does not itself enforce a linter denying a new `*.py` or Cargo dependency, execute tests, delete old code, validate absence of active Python processes, or prove production readiness. A future PR gate may be proposed as a separate guarded source/CI change.
