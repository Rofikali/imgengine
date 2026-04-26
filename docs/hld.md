# High-Level Design (HLD) — imgengine

## Purpose

This document summarizes the high-level architecture of imgengine based on the code in `src/` and the public/internal headers in `include/`. It is intended to describe components, responsibilities, runtime flow, and extension points so engineers can quickly understand system boundaries.

## Overview

imgengine is a modular, kernel-grade image processing engine implemented in C and built with CMake. The repository separates stable/public APIs (`include/api/v1` and `src/api`) from internal implementation (`include/*` and `src/*`). The system is designed for high throughput and low latency with explicit hot/cold code separation, architecture-specific kernels, and a plugin ABI.

## Key Components

- **API:** `include/api/v1` and `src/api` — stable user-facing ABI for creating and submitting image jobs, managing buffers, handling errors, and reading results.
- **Core:** `include/core`, `src/core` — central data structures, context and lifecycle management, buffer lifecycle, result types.
- **Pipeline:** `include/pipeline`, `src/pipeline` — pipeline builder, DAG orchestration, jump-table based execution, and kernel fusion infrastructure.
- **IO:** `include/io`, `src/io` — decoders/encoders, streaming decoders, and VFS adapters (posix, memory, s3, http). Also includes `third_party/stb` bridge.
- **Memory:** `include/memory`, `src/memory` — slab and arena allocators, hugepage support, NUMA-aware allocation and aligned allocators.
- **Runtime:** `include/runtime`, `src/runtime` — worker threads, queues (SPSC/MPMC), scheduler, RPC stubs for distributed/cluster use, affinity and backpressure.
- **Plugins:** `include/plugins`, `src/plugins`, `src/runtime/plugin_loader.c` — plugin ABI, registration and static plugin implementations (resize, crop, grayscale, pdf, bleed).
- **Arch layer:** `include/arch`, `src/arch/*` — arch-specific implementations (x86_64 AVX2/AVX512, scalar, aarch64/neon). Jump-table selects appropriate kernel at runtime.
- **Observability:** `include/observability`, `src/observability` — logging, binlog, metrics, tracing, and profiler.
- **Security:** `include/security`, `src/security` — input validation, bounds checks, sandboxing seccomp integration.
- **Cmd / Startup:** `src/cmd/imgengine` and `src/startup` — CLI entrypoint (`main.c`), job_builder, engine initialization and subsystems bootstrap.

## Data Flow (high-level)

1. Input (file/HTTP/S3/memory) is fetched by VFS adapters in `src/io/vfs`.
2. Decoder(s) in `src/io/decoder` convert raw bytes to internal slab buffers.
3. API constructs a `job` and submits it (`src/api/*`).
4. Scheduler enqueues jobs into worker queues (`src/runtime/*`).
5. Worker executes pipeline via the jump-table / fused kernels path (`src/hot`, `src/pipeline/*`).
6. Processed data is handed to encoder / output path (`src/io/encoder/*`).
7. Observability hooks/logging emit metrics and traces; job finalization returns results via API.

## Build & Configuration Notes

- Build system: CMake (see `CMakeLists.txt`). Primary options discovered in the codebase include `IMGENGINE_LTO`, `IMGENGINE_SANITIZE`, `IMGENGINE_SANDBOX`, and `IMGENGINE_BENCH`.
- Architecture detection toggles path under `src/arch/*` (AVX2/AVX512/Scalar/AArch64), and the build tree includes explicit lists of sources by component.
- Optional dependency: libjpeg-turbo (auto-fetched when not found on system)

## Runtime Considerations

- NUMA and hugepage aware allocators used to minimize cross-socket contention and TLB pressure.
- Worker threads are pinned and scheduled with backpressure to avoid overload.
- Hot code (SIMD kernels, fused kernels) lives in `src/hot` and `src/arch/*` and must be kept branchless and inlinable where possible.
- Cold code handles validation, errors, and debugging; it intentionally trades latency for clarity.

## Extension Points

- Plugins: `include/pipeline/plugin_abi.h` and `src/runtime/plugin_loader.c` define plugin interface and loading logic.
- Pipeline DSL/codegen: The CMake option `IMGENGINE_ENABLE_DSL_CODEGEN` generates `src/pipeline/generated.c` from presets and templates in `include/pipeline`.
- Adding kernels: implement new arch kernels under `src/arch/<arch>/` and register them via `hardware_registry`.

## Observability & Testing

- Metrics, tracing and binlog support are available in `src/observability` and headers in `include/observability`.
- Bench tooling is under `src/cmd/bench` and the build toggles `IMGENGINE_BENCH`.

## Security Model

- Input validation and bounds checks are centralized in `src/cold/validation.c` and `include/security` headers.
- Optional seccomp sandbox (controlled by `IMGENGINE_SANDBOX`) is configured at startup in `src/startup`.

## Open Questions / Next Steps

- Verify and document the stable public API surface under `include/api/v1` (versioning policy, semantic stability guarantees).
- Produce a function-level LLD (this repository's `lld.md`) mapping important public functions to implementing files and call graphs.
- Add sequence diagrams for common flows (CLI job -> decode -> pipeline -> encode).

---

Files referenced during this analysis: `include/`, `src/`, and `CMakeLists.txt` (see repo for details).

For a deeper low-level mapping, see `docs/lld.md`.

## Hard Rules (Non‑Negotiable)

- **Downward dependencies only:** higher-level modules must not be referenced by lower-level modules. Keep public API -> core -> arch direction strictly enforced.
- **Hot-path constraints:** no `malloc`/`free`, no blocking I/O, no locks or blocking syscalls, and avoid data-dependent branching in hot kernels. Hot code must be deterministic and side-effect free where feasible.
- **Pipeline purity:** pipeline operators should be pure (no global state mutation) or otherwise have well-defined ownership/contract semantics documented in headers.
- **Zero-copy contracts:** use slab/arena ownership semantics (`img_slab_alloc` / `img_slab_recycle`) — clearly document ownership transfer on API boundaries.
- **Init order guarantees:** ensure `img_jump_table_init` runs before architecture/hardware registration and any `img_register_op` plugin calls (done during deterministic startup / `img_api_init`).
- **Fail-fast in hot path:** detect and surface errors to cold-path handlers rather than attempting expensive recovery inside kernels.
- **Stable ABI:** public headers under `include/api/v1` are versioned; require symbol/version checks during plugin registration.

## Kernel-grade Design Patterns (recommended)

Below are concise, actionable patterns that map well to the existing codebase and the constraints of kernel-grade, performance-sensitive systems.

- **Strategy:** runtime selection of algorithmic variants (used by jump-table selection of `img_arch_*` kernels). Use a light-weight strategy table (function pointers) rather than virtual dispatch.
- **Factory / Registration:** register platform-specific kernels at startup (`hardware_registry` / `img_register_op`) so callers request an opcode and receive the best available implementation.
- **Command / Pipeline (Chain-of-Responsibility):** represent pipeline ops as compact command structures (`op_code`, params, buffer refs). This makes queuing, serialization, replay, and batching efficient.
- **Adapter / Bridge:** adapt third‑party decoders/encoders via small vtables (`img_io_vtable`) to isolate external API changes from core runtime.
- **Facade:** present a minimal, stable façade for API clients while hiding complex startup/engine lifecycle and tuning knobs.
- **Observer / Event Hooks (low-overhead):** expose sampling hooks for metrics/traces; keep fast-path hooks conditional and inexpensive.
- **State Machine:** model job lifecycle explicitly (`created` → `prepared` → `scheduled` → `running` → `completed`) to enforce invariants and idempotence.
- **Data-Oriented Design (DOD):** favor contiguous memory, struct-of-arrays, and layout optimizations to maximize SIMD utilization and prefetch efficiency.
- **Lock-free messaging + work-stealing:** use per-worker SPSC queues for the hot path, an MPMC fallback for async submission, and a work-stealing deque (or light-weight steal API) for load balancing.
- **Scoped/RAII-like cleanup (C idiom):** use explicit `recycle`/`free` semantics and `cleanup` macros where convenient; avoid implicit frees in hot path.
- **Versioned Plugin ABI & Compatibility Checks:** embed numeric version checks into plugin registration to prevent runtime mismatches.

## Implementation Guidance — mapping patterns to repository primitives

- Hot dispatch: keep `g_jump_table[op]()` direct and inlinable — this is the hot lookup (see `src/pipeline/jump_table_register.c`).
- Kernel registration: implement platform-specific factories in `src/pipeline/hardware_registry.c` and generated registrations under `src/pipeline/generated.c`.
- IO adaptors: implement `img_io_vtable` for decoder/encoder adapters and pass vtables during initialization for testability.
- Memory management: use `img_slab_alloc` / `img_slab_recycle` for transient buffers; prefer pooling to avoid allocator churn in the hot path.
- Scheduler: per-worker SPSC + global MPMC submit + work-steal (`img_scheduler_steal`) for resiliency and low latency.
- Observability: use lightweight sampling and per-job trace ids; avoid heavy logging in hot loops (`img_log_write` should be cold-path only or rate-limited).

## Review / CI Checklist (kernel-grade PR review)

- Build with `IMGENGINE_SANITIZE` and run unit tests.
- Run lint/format and static analyzers (UBSan/ASan) as gating checks.
- Verify no heap allocations occur in hot path using instrumentation or targeted sanitizers.
- Add microbenchmarks for any changed kernel and include regression thresholds in CI.
- Add ABI checks (compare `nm` exports to canonical CSV/JSON) when touching `include/api/v1` or registration code.

## Next steps

- Expand `docs/design_patterns.md` with small code sketches mapping each pattern to repo examples and test harnesses.
- Optionally produce a compact checklist template for reviewers to enforce the Hard Rules on PRs.

## Problem → Pattern Map

Below are common problems you will encounter while working toward the RFC targets, the design pattern to think of first, why it helps, and quick repo examples to inspect.

- **Multiple algorithms interchangeable:** *Strategy* — pick the best kernel at runtime (CPU caps, image size). See `src/pipeline/jump_table.c` and `src/pipeline/hardware_registry.c`.
- **Object creation complex:** *Factory* — centralize creation/initialization (pipelines, engine contexts, decode/encode workers). See `src/pipeline/pipeline_executor.c` and `src/api/api_init.c`.
- **One-to-many updates (metrics/traces):** *Observer* — decouple instrumentation from core logic with low-overhead hooks. See `src/observability/logger_write.c` and `src/observability`.
- **Add behavior without changing code:** *Decorator* — wrap kernels/pipelines for caching, validation or sampling. Inspect `src/pipeline/generated.c` and `src/runtime/plugin_loader.c` for registration/decorators.
- **Central shared instance:** *Singleton* — global registries like `g_jump_table` and `g_io_vtable` must have deterministic init and clear lifecycle. See `src/pipeline/jump_table_register.c` and `src/io/io_register.c`.
- **Layered access control for IO:** *Proxy* — wrap remote VFS/IO with caching, auth, rate-limiting and retries. See `src/io/io_register.c` and `src/io/decoder/*`.

### Bench-derived problems (quick read)

- **Decode ingress is dominant (bench):** pattern candidates: Strategy (choose streaming vs fast decoder), Factory (decoder factory), Proxy (cache remote fetch), Pipeline (pre-decode prefetch + parallel decode).
- **Encode is significant:** pattern candidates: Pipeline (encode worker pool), Strategy (select tuned encoder), Decorator (post-processing offload).
- **Throughput low vs RFC:** pattern candidates: Lock-free messaging + work-stealing, Data-Oriented Design for hot kernels, batch fused kernels (Strategy).

## Actionable next steps (short)

- Implement a small decoder `Factory` + `Strategy` test: add a config flag to select streaming vs bulk decoder; microbench decode-only paths.
- Add a `Decorator` wrapper type for kernels to attach caching/validation without touching core kernels.
- Create `docs/design_patterns.md` with code sketches and a checklist for reviewers enforcing Hard Rules (I can generate this next).


