# imgengine

**Product direction (2026-11-16):** Owner-selected [IMGENGINE Free, Pro, Studio and Developer API portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md). **Free first** ships only **Crop, Resize, Compress, Rotate and Convert**, browser-local where supported and no image upload by default. Pro (creator recipes/batch), Studio (print/operator workflows) and Developer API (future metered Rust/C17 service) are **planned product lines, not live subscriptions or APIs**. See [current F1 and later NOW/NEXT/LATER milestones](next.md), [future API billing/capacity design](docs/IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md) and [business/security/release gates](docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md). F0 strategy is merged; the five free tools, paid tiers and public releases still need implementation and independent approval.

**New repository-root command menu (review proposal):** Run `make help` and `make self-check` from the project root in Ubuntu 24.04/Linux for safe C17/Rust/Nuxt/Docker build and test targets. See [Root Makefile Engineering Guide](docs/ROOT_MAKEFILE_ENGINEERING_GUIDE.md) for focused commands, prerequisites, full-verifier opt-in, log budgeting and risks. Existing `imgengine/Makefile` remains native-only; `make` at root is deliberately help-only. This does not replace seven-job GitHub CI or production release gates.

> **Technology decision (2026-11-14):** All **new** IMGENGINE backend code uses **Rust/Axum**, with C17 native execution, safe Rust FFI and the existing Nuxt/Vue/TypeScript frontend. **No new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, or Python tooling.** The tracked Python-based backend and older helper scripts are legacy/existing and retained only until verified migration or tooling replacement. The current Rust backend lives in `backend/`; `imgengine-saas/backend/` is **legacy Python**. Read [approved technology/folder ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) before proposing new infrastructure or folder moves.

**Infrastructure reviewers:** [infra AGENTS](infra/AGENTS.md), [infra README](infra/README.md), and [2026-11-15 infrastructure audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) define current Caddy/Docker/secret/observability and VPS-release evidence boundaries. This does not authorize production changes.

**C17 native-core deep audit:** [Principal source/LLD/design patterns/DSA/security review](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) and [native core README](imgengine/README.md) identify implemented ABI/algorithms and high-priority concurrent SPSC consumer/metrics and resource-budget questions. These require exact-revision proofs, **not** broad rewrites or unverified nanosecond claims.

**Starting Astra or another engineering agent?** Read [Astra engineering start guide](docs/ASTRA_ENGINEERING_START_HERE.md) before choosing frontend, Rust backend or C17 native work. [next.md](next.md) is the living status board; current scoped [Rust AGENTS](backend/AGENTS.md), [native AGENTS](imgengine/AGENTS.md) and [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) define boundaries. Merged PR #37 relocated Rust to `backend/` with [seven passing CI jobs](https://github.com/Rofikali/imgengine/actions/runs/43618598771), **not** a production release.

## Current application path

The verified synchronous, ephemeral application path is:

```text
Browser -> Nuxt server proxy -> Rust/Axum API
        -> RequestLifecycleService -> bounded Rust admission
        -> safe Rust FFI -> libimgengine C ABI -> C scheduler -> C engine
        -> completed JPEG response -> cleanup
```

Implemented Rust endpoints cover basic render, FIT-only layout render, and the
physical-sheet render slice. Rust owns authentication, validation, lifecycle,
bounded admission, response abandonment, graceful draining, and cleanup; C
retains native scheduling and image execution.

The older FastAPI/Celery/Redis/PostgreSQL application remains in the repository
as a legacy/rollback path. It is not the target first-MVP deployment path and
has not been silently deleted or declared retired. See
[`docs/api.md`](docs/api.md), [`docs/architecture.md`](docs/architecture.md),
and [`docs/MVP_RELEASE_CHECKLIST.md`](docs/MVP_RELEASE_CHECKLIST.md) for the
current contract, migration status, and open release gates.

The scoped frontend and retained Python backend engineering records are indexed in the [frontend/legacy backend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md). Follow the respective `imgengine-saas/frontend/AGENTS.md` and `imgengine-saas/backend/AGENTS.md`; the active Rust implementation follows `backend/AGENTS.md`.

For a local Windows 11/Docker Desktop rehearsal of the current Rust/Nuxt P12
candidate, use [`docs/LOCAL_DOCKER_REHEARSAL.md`](docs/LOCAL_DOCKER_REHEARSAL.md).
It is local integration evidence only, not actual-VPS or production evidence.

> A native, high-performance image layout engine for print-ready photo sheets — built in C with a stable public API and a Rust-backed application path.

**License/provenance:** No root `LICENSE` file was present in the audited Git tree. The earlier MIT badge was removed pending an explicit, owner-approved license and third-party provenance decision; this statement grants no license. See [project status](docs/PROJECT_STATUS_AND_DECISION_REGISTER.md).
[![Language](https://img.shields.io/badge/core-C17-00599C.svg)](https://en.wikipedia.org/wiki/C17_(C_standard_revision))
[![Build](https://img.shields.io/badge/build-CMake-064F8C.svg)](https://cmake.org/)
[![Focus](https://img.shields.io/badge/focus-systems%20%2B%20image%20processing-111827.svg)](#)

## Why imgengine?

`imgengine` is designed around a simple engineering boundary:

**the native engine owns image processing; the application layer owns product concerns.**

The native core handles decoding, geometry, layout, rendering and encoding. Authentication, tenant policy, uploads, persistence, retries and billing belong outside the engine.

That separation makes the project useful both as a native library/CLI and as the processing core of a future SaaS product.

## What it does

- Decode supported source images
- Convert physical print dimensions (cm + DPI) into pixel dimensions
- `FIT` or `FILL` image placement
- Grid-based sheet composition
- Borders, bleed and crop marks
- PNG/JPEG/PDF output paths
- CLI and public C API
- RGB24/raw ingress for already-decoded frames
- SIMD-accelerated paths with portable fallbacks
- Bounded resource validation for untrusted input

## Architecture

```text
                         Product / SaaS Layer
                                  │
                         ┌────────▼────────┐
                         │ Rust Backend     │
                         │ API / policy /   │
                         │ lifecycle /      │
                         │ observability    │
                         └────────┬────────┘
                                  │ stable FFI
                         ┌────────▼────────┐
                         │ imgengine Core   │
                         │ C17             │
                         ├─────────────────┤
                         │ decode          │
                         │ resize / crop   │
                         │ layout          │
                         │ render          │
                         │ encode          │
                         │ SIMD dispatch   │
                         └─────────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    ▼                           ▼
                    JPEG                      PDF
```

The native specification defines the processing contract and acceptance matrix in [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md).

## Processing pipeline

```text
Input
  │
  ▼
Decode / validate
  │
  ▼
FIT or FILL
  │
  ▼
Grid placement
  │
  ▼
Border
  │
  ▼
Bleed + crop marks
  │
  ▼
Encode
  │
  ▼
Print-ready output
```

## Native API

The public C API exposes an explicit engine lifecycle and job execution boundary, including file-to-file, raw encoded output and RGB24 ingress paths.

The project treats ABI stability as an engineering contract: public symbols, ownership, lifecycle, threading and release semantics are tested rather than left implicit.

## Build

### Requirements

- CMake 3.16+
- GCC or Clang
- Linux (primary deployment target)
- NASM for JPEG SIMD dependency builds where required

```bash
git clone https://github.com/Rofikali/imgengine.git
cd imgengine

cmake -S . -B build
cmake --build build -j
```

Run the CLI:

```bash
./build/imgengine_cli --help
```

## Example

```bash
./build/imgengine_cli \
  --input photo.jpg \
  --output sheet.jpg \
  --cols 6 \
  --rows 6 \
  --width 4.5 \
  --height 3.5 \
  --dpi 300 \
  --bleed 10 \
  --crop-mark 20
```

## Engineering quality

Correctness is treated as a first-class feature. The native acceptance matrix covers:

- deterministic geometry and layout fixtures
- FIT vs FILL behavior
- border, bleed and crop-mark behavior
- layout-boundary/property testing
- malformed-input and unsafe-dimension handling
- SIMD vs scalar equivalence
- ASan/UBSan validation
- bounded fuzzing of untrusted parsing/decoder paths
- memory and arithmetic boundary tests

See [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md) for the detailed contract.

## Project direction

The project is evolving from a native image-processing engine toward a low-cost, secure SaaS architecture:

```text
C native engine
      ↓
stable C ABI
      ↓
Rust FFI boundary
      ↓
Rust service / lifecycle orchestration
      ↓
Nuxt + TypeScript frontend
```

The design deliberately avoids making the native engine responsible for authentication, billing, persistence or product policy.

## Repository structure

```text
imgengine/
├── include/          # public/internal C interfaces
├── src/              # native implementation
├── tests/             # native correctness and security tests
├── docs/              # specifications and verification evidence
├── imgengine-saas/    # application/SaaS layer
├── CMakeLists.txt
└── README.md
```

## Status

The native ABI, safe Rust FFI, Rust lifecycle/admission boundary, Axum adapter,
and initial Nuxt server proxies are implemented and verified in Ubuntu 24.04
Docker CI. Production readiness, actual-VPS evidence, canary operation, and
legacy-stack retirement remain open release decisions. A passing repository or
Docker gate is not a production-readiness claim.

## License

MIT
