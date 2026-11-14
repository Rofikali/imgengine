# Rust backend relocation — implementation and review contract

**Status:** **CODE REFACTOR IN DRAFT** until exact-revision CI succeeds and the pull request is approved/merged. The same Rust crates remain; this is **not** a new service, API behavior change, deployment or optimization. The migration is based on `main` commit `60b857405c434b77072f53bf4c50a53ae82cba12` (after technology-policy PR #36).

## Design and approved ownership

The existing `imgengine/rust/` Rust subtree was moved to **top-level `backend/`** so the Rust/Axum application and its safe C FFI are under an unambiguous backend directory. Native C17 stays in `imgengine/`; Nuxt/Vue/TS stays in `imgengine-saas/frontend/`. Existing `imgengine-saas/backend/` remains **legacy Python/FastAPI/Celery/Redis/PostgreSQL** and was neither moved nor deleted.

```text
backend/
  AGENTS.md
  Dockerfile
  docker-entrypoint.sh
  imgengine-api/               # Axum HTTP; P5/P8/P12
  imgengine-supervisor/        # bounded Rust admission/lifecycle
  imgengine/                   # safe C ABI FFI wrapper
  imgengine-ffi-smoke/         # FFI consumer smoke
imgengine/
  AGENTS.md
  include/imgengine/api/       # versioned C ABI
  src/                         # C17 native engine/scheduler
imgengine-saas/
  frontend/                    # existing Nuxt server proxy and UI
  backend/                     # existing legacy Python ONLY
```

All **23 original tracked Rust subtree files** are moved with their original Git blob contents **except**: (1) `backend/Dockerfile`, updated for root Docker context and new manifest path; (2) `backend/AGENTS.md`, updated for its new scope and links; (3) `backend/imgengine-api/src/lib.rs`, whose `include_bytes!` test fixture path was corrected from `../../../tests/fixtures/` to `../../../imgengine/tests/fixtures/`. Rust API request/response/runtime logic is intentionally unchanged.

## Updated dependency and deployment paths

| Consumer | Before | After |
| --- | --- | --- |
| Rust HTTP crate | `imgengine/rust/imgengine-api/` | `backend/imgengine-api/` |
| Rust supervisor | `imgengine/rust/imgengine-supervisor/` | `backend/imgengine-supervisor/` |
| Safe Rust FFI crate | `imgengine/rust/imgengine/` | `backend/imgengine/` |
| Rust smoke | `imgengine/rust/imgengine-ffi-smoke/` | `backend/imgengine-ffi-smoke/` |
| API Dockerfile / entrypoint | `imgengine/rust/Dockerfile` and `imgengine/rust/docker-entrypoint.sh` | `backend/Dockerfile` and `backend/docker-entrypoint.sh` |
| API Docker build context | `imgengine/` | repository root; `COPY imgengine` and `COPY backend` |
> **Verification incident on earlier PR head (resolved in branch source, new checks still required):** The first comprehensive Ubuntu 24.04 Docker run failed **after native tests and Rust API tests passed** because `imgengine/tests/abi/real_image_verification.sh` still referenced `$source_dir/rust/imgengine-ffi-smoke/Cargo.toml`. The tested revision was superseded. The script was corrected to `$source_dir/../backend/imgengine-ffi-smoke/Cargo.toml`; this is a test-harness path fix, not a native C ABI or Rust rendering change. Six other checks completed successfully on that old revision; **do not claim their results validate the subsequent commit**. Re-run required CI at the new head before merging.

| Build helper manifests | `$source_dir/rust/...` | `$source_dir/../backend/...` |
| CI / local candidate API image | old API Dockerfile and native-only context | root-context backend Dockerfile |
| Legacy Compose Rust API | `context: ../../imgengine`, `rust/Dockerfile` | `context: ../..`, `backend/Dockerfile` |

All four Cargo packages remain siblings: the local `../imgengine` and `../imgengine-supervisor` path dependencies should remain valid. Build output is still the `imgengine-api-server` binary, and the runtime image still requires `libimgengine.so` and its external dynamic dependencies. No broad `unsafe`, `Send`/`Sync`, memory, HTTP, format or scheduler changes are authorized.

## Release/check gates — **not yet executed or passed by documentation**

- **Cargo path resolution:** `cargo metadata --format-version 1 --manifest-path backend/imgengine-api/Cargo.toml`; verify local dependency manifests, lockfile `--locked` and fixture `include_bytes!` path.
- **Native C / Rust FFI:** run the authoritative Ubuntu Docker verification, C11/C17 compatibility, API Rust tests, native ABI consumer, sanitizer/fuzz/ffi gates required for a native release.
- **Docker image:** `docker build --file backend/Dockerfile --tag imgengine-rust-api:layout-check .`; verify build stage receives both C core and Rust crates, non-root runtime, `ldd` and healthcheck.
- **Infrastructure:** verify P7 Docker dev compose, P8 restricted build, P12 disposable candidate, CI Rust API container and the retained legacy Compose Rust service; check secrets remain server-only.
- **Product parity:** P5/P8/P12 endpoints, bounded admission, P12 exact sheet counts, safe errors, request body caps, output ownership and cleanup must match original behavior.
- **Documentation and configuration:** no broken linked paths; older revision-bound evidence retains the original historical paths. Do not use this PR to reclassify P13 crop, P14 actual VPS or paid tenant functionality as finished.
- **Business/governance:** quantify only actual developer-onboarding and maintenance benefits, not false nanosecond/image throughput gains. Preserve original checkout and rollback via normal Git revert of a **single structural PR** if CI or deployment evidence fails.

## Exact status

| Item | Status on this feature branch |
| --- | --- |
| Rust crates physically in root `backend/` | **MOVED IN PR**, not yet merged |
| Code logic/API behavior | **INTENDED UNCHANGED**; tests still required |
| C core and ABI | **UNCHANGED** |
| Nuxt frontend and typed server proxies | **UNCHANGED** |
| Legacy Python backend and old DB/queues | **RETAINED / NO CHANGES** |
| Build/CI/Compose paths | **UPDATED**, exact CI still required |
| P13 crop integration | **HALF IMPLEMENTED**, unaffected |
| P14 VPS public release | **BLOCKED**, unaffected |
| Static public `.a` SDK | **NOT IMPLEMENTED**, unaffected |
| Production deployment | **NOT PERFORMED** |

**Authority:** [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [backend AGENTS](../backend/AGENTS.md), [root AGENTS](../AGENTS.md), [native C AGENTS](../imgengine/AGENTS.md), [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [root next queue](../next.md), [Rust API contract](api.md), [scoped testing](CHANGE_SCOPED_VERIFICATION.md). Follow exact source and tests, not the convenience of a directory name.
