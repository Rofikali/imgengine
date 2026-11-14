# IMGENGINE Rust backend

**Status:** directory layout change in [migration review](../docs/RUST_BACKEND_DIRECTORY_MIGRATION.md). This is the active **Rust/Tokio/Axum backend** source tree. It does **not** contain or depend on the retained Python/FastAPI service in `imgengine-saas/backend/`.

## Crates

| Directory | Responsibility |
| --- | --- |
| `imgengine-api/` | Versioned Axum routes, validation, error response and server executable |
| `imgengine-supervisor/` | Bounded admission, request lifecycle, shutdown/drain and native-call orchestration |
| `imgengine/` | Safe Rust facade over the installed C17 public ABI; narrow unsafe FFI layer |
| `imgengine-ffi-smoke/` | External Rust consumer/ABI smoke check |

The native C17 engine is under [`../imgengine/`](../imgengine/), its versioned public headers under [`../imgengine/include/imgengine/api/`](../imgengine/include/imgengine/api/). The active Nuxt UI lives under [`../imgengine-saas/frontend/`](../imgengine-saas/frontend/).

## Engineering authority

Read [backend AGENTS.md](AGENTS.md), [root AGENTS.md](../AGENTS.md), [native C ABI rules](../imgengine/AGENTS.md), [approved stack](../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [Rust API contract](../docs/api.md), [Rust lifecycle](../docs/RUST_REQUEST_LIFECYCLE.md), [scoped verification](../docs/CHANGE_SCOPED_VERIFICATION.md), and [next.md](../next.md) before changing the backend.

- **New backend work is Rust**, not Python/FastAPI/Celery/Redis/PostgreSQL.
- This directory contains several **separate Cargo packages**, not a Cargo workspace at `backend/Cargo.toml`. Each manifest uses sibling path dependencies. Do not assume a root `cargo test --workspace` works.
- The C engine remains the owner of decode, transform, layout, SIMD and native scheduling. Rust owns HTTP/admission, errors and scoped product policy.
- C FFI ownership, lifecycle, native cancellation limitations, memory envelope and output release are fixed by the versioned contracts. Do not invent `Send`/`Sync` or silently add worker/DB infrastructure.
- A directory relocation alone does **not** improve latency, security or capacity; independently verify those claims.

## Reproducible commands (from repository root)

Inspect the manifest and dependency topology:

```bash
cargo metadata --format-version 1 --manifest-path backend/imgengine-api/Cargo.toml
```

Build the release API image using the repository root as Docker context:

```bash
docker build --file backend/Dockerfile --tag imgengine-rust-api:layout-check .
```

For local Rust tests, build or obtain the **already validated compatible native `libimgengine.so`** and set `LD_LIBRARY_PATH` and `RUSTFLAGS=-L native=...` before running a crate's `cargo test --manifest-path backend/.../Cargo.toml`. The repository's [Linux native verifier](../imgengine/scripts/verify_linux.sh) demonstrates the required native/FFI environment and the comprehensive release gates. Do not manually rerun that complete C sanitizer/fuzz matrix on ordinary isolated Rust API documentation or frontend-only work.

The Docker build and full CI must pass on this exact PR head before merge; historical tests of the old source layout do not validate a relocation. The [P14 real-host deployment gate](../docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) is still open.

## Migration/rollback

The old `imgengine/rust/` source path is removed from this feature branch. External tooling must update manifest and Docker paths, not duplicate crates into both directories. [Migration contract](../docs/RUST_BACKEND_DIRECTORY_MIGRATION.md) lists all changed integration points and rollback gates. The old Python backend and its persistent data are **unchanged**.
