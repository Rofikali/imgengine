# AGENTS.md — IMGENGINE Rust API, supervision and safe FFI

**Scope:** top-level `backend/` and its descendants. This file supplements [root AGENTS](../AGENTS.md) and [native C/FFI AGENTS](../imgengine/AGENTS.md). On conflict, preserve stronger security and ABI invariants; consult the [approved technology and structure ADR](../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md).

## 1. Approved backend: Rust, not Python

**All NEW backend services, HTTP APIs, application logic, workers, supervisor code, integrations and related test utilities must use Rust** (primarily Tokio/Axum where applicable). Do **not** add Python, FastAPI, Flask, Django/DRF, Celery, Redis, PostgreSQL, or other Python/broker/DB dependencies as shortcuts. No new Python scripts/tools. Existing Python code and some historical verification scripts elsewhere in the repository are **legacy/existing**, not part of this directory's implementation mandate. Maintenance or retirement of those files is a separate approved change; this policy does not claim they have been removed.

Do not introduce a new database, queue, ORM, object store, microservice, Kubernetes workload or paid provider because it appears customary in SaaS tutorials. For a future paid/customer-account requirement that genuinely needs durable metadata, request a new user-approved storage ADR first. **PostgreSQL and Redis are not default or fallback options.**

## 2. Responsibility map

| Crate | Owns | Must not own |
| --- | --- | --- |
| `imgengine-api/` | Axum HTTP adapters, transport validation, backend-only API keys, safe status/problem responses | Direct pixel kernels, leaking native pointers, permanent image jobs |
| `imgengine-supervisor/` | Request lifecycle, bounded admission, overload, deadlines, disconnect/drain semantics, ephemeral cleanup | Duplicate C native scheduler, Redis/Celery queue |
| `imgengine/` | Safe ownership facade to public C ABI, narrow isolated `unsafe` declarations, native status mapping | HTTP, SaaS pricing, accounts, databases |
| `imgengine-ffi-smoke/` | External consumer/link integration smoke for documented native ABI | Application-specific product features |
| `../imgengine/src/`, `../imgengine/include/` (native C subtree) | C17 codec, layout, SIMD, scheduler, versioned C ABI | Product billing, HTTP and user identity |

The Rust/C bridge is `HTTP → request lifecycle → bounded admission → safe Rust facade → stable C ABI → C scheduler`. Keep separate transport/application/native boundaries and exact versioned contracts, rather than blindly implementing all DDD/Hexagonal/GoF patterns.

## 3. Security and correctness

- Treat all HTTP and encoded image bytes as hostile; check input bytes, decoded dimensions, geometric arithmetic, output size, CPU time, memory and concurrent admission at the appropriate layer. A bounded encoded JPEG destination is **not** a bound on total process RSS.
- Keep `unsafe` small, document pointer/ownership and out-parameter invariants, use RAII for native output, and never invent an `unsafe impl Send`/`Sync` for an opaque C engine.
- Preserve the process-/engine-wide operation serialization currently in the native API. Rust admission does not replace C scheduling.
- Do not advertise native cancellation: a Rust request timeout/disconnect may abandon the response but does not stop an already running ABI v1 native function.
- No user image bytes, credentials, filenames, private paths, raw exception details or secrets in logs, metrics, traces or analytics. Use generated correlation IDs and bounded, low-cardinality telemetry.
- API keys are server-to-server credentials in the current slice, **not** complete tenant accounts or a billing/authorization service.
- Use error classes, ownership semantics, exact geometry and cleanup from [current Rust API contract](../docs/api.md), [request lifecycle](../docs/RUST_REQUEST_LIFECYCLE.md), [safe FFI](../docs/RUST_FFI.md), [P12](../docs/P12_PHYSICAL_SHEET_CONTRACT.md) and [P13](../docs/P13_INTERACTIVE_CROP_CONTRACT.md). Do not copy outdated legacy Python job/queue claims.

## 4. Tests, efficiency, and evidence

- Begin with the exact diff and impacted Rust/ABI boundary. For Rust-only HTTP/lifecycle changes, run focused `cargo test`/format/lint/security and relevant HTTP/cleanup tests. Do **not** manually rebuild/retest C or run full ASan/UBSan/native fuzz unless C/FFI/ABI/native behavior changed.
- For a C ABI/FFI ownership change run focused Rust consumer + ABI/ownership/layout tests and relevant native security gates, broadening to the required matrix before merge/release. Never bypass required GitHub Actions checks.
- For docs-only work, check scope and Markdown links; no need to compile or run native fuzz.
- Report exact tested SHA, commands and pass/fail; store full logs outside the agent context. Previous recorded tests are not new-SHA evidence.
- Performance: measure end-to-end Rust HTTP, admission, native decode/render/encode, output copy and RSS separately. Tiny functions may be nanoscale; images generally take more time. Security and output parity always win over a benchmark.

## 5. Task and decision handoff

Read [root next.md](../next.md), [approved stack ADR](../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [native contract matrix](../docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) and optional [skills.md](../skills.md) before each milestone. M1.1 P13 user-crop Rust FFI/API integration is **planned**, while native C crop ABI 2.3 is implemented. Do not invent supported new routes or formats.

For every PR, update only the affected status/evidence in `next.md` and the canonical contract. New technology adoption, legacy retirement, filesystem move, public deployment and a paid data store require their own explicit user decision. Preserve current source directories and local dirty worktrees.
