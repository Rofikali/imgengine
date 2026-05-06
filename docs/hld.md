# High-Level Design (HLD) — imgengine (Production-ready)

This HLD captures the v2.0 RFC's intent and translates it into a concrete, reviewable architecture suitable for a kernel-grade, deterministic execution engine. It is meant to be the authoritative reference for system-level decisions, runtime constraints, and production policies.

## 0. System Definition (one line)

`A Kernel-Grade, Deterministic, Data-Oriented, SIMD-Fused, Zero-Allocation Execution Engine with Auto-Generated Pipelines`

## 1. Goals & Targets

- Latency: < 2 ms (4K image) for render-only paths
- Throughput: 100K+ ops/sec (scale-out and per-socket targets)
- Hot-path: zero heap allocation, branchless SIMD fusion where possible
- Deterministic execution and reproducible latency profiles

### Non-Goals

- GPU-first execution
- Desktop GUI tooling or general-purpose interactive editing

## 2. Architecture Overview

High level layers (strict downward dependencies):

- Control Plane: `cmd` → `api` → `runtime`
- Execution Plane: `pipeline` → `arch` → `memory` → `core`

Execution is modelled as a strict DAG from job submission to SIMD fused kernel execution and output.

See repository layout for canonical locations: [CMakeLists-Instructions.md](CMakeLists-Instructions.md) and the `src/` layout.

## 3. Execution Flow (Strict DAG)

```
USER
 ↓
cmd
 ↓
api          ← validation + IO boundary
 ↓
runtime      ← scheduler (brain)
 ↓
pipeline     ← PURE execution
 ↓
arch         ← SIMD fused kernels
 ↓
memory       ← slab / arena
 ↓
core         ← POD types only
```

Key rules: API is the only IO boundary, runtime performs scheduling, pipeline must be pure (no side-effects), and arch kernels implement validated leaf operations only.

## 4. Hard Rules (Production Policies)

- Downward dependencies only.
- No `malloc`/`free` in hot path; slab/arena allocation only at startup or API boundaries.
- No IO or syscalls in hot path.
- No locks in hot path; prefer lock-free SPSC/MPMC primitives.
- Pipeline operators are pure or otherwise have clearly documented ownership semantics.
- Runtime manages prepared registries and prepared decode state; pipeline execution must not perform control-plane work.

Enforcement: CI gates will fail builds when these policies are violated (see CI/CD section).

## 5. Control vs Execution Plane

- Control Plane: `src/cmd`, `src/api`, `src/runtime` — parsing, validation, IO, template resolution, and scheduling.
- Execution Plane: `src/pipeline`, `src/arch`, `src/hot` — pure SIMD fused kernels, jump-table dispatch, and memory-local execution.

Control plane can allocate, perform logging, and make blocking calls. Execution plane must be deterministic and allocation-free.

## 6. Prepared Registry & Decode Path

The runtime owns a prepared template registry and prepared decoder objects. These are created at startup or during a controlled prepare phase and stored as runtime-owned objects referenced by jobs.

Prepared registry responsibilities:

- Template key → prepared job entry mapping (constant-time lookup)
- Job defaults and presets are centralized; runtime resolves presets first and then applies overrides
- Prepared decode state (thread-local slab + arena) to avoid repeated heavy initialization

Pseudo flow:

```
CLI/API Request
	↓
TemplateKey
	↓
Runtime Template Registry
	↓
Prepared Job Entry
	↓
Prepared Decode State
	↓
Pipeline / Arch Dispatch
```

Files: [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L1), [src/pipeline/fused_registry.c](src/pipeline/fused_registry.c#L1)

## 7. Raw Frame Ingress (decode bypass)

Raw RGB24 frames can be submitted explicitly via a raw ingress contract. Rules:

- Raw ingress is explicit (never autodetected).
- Width/height/stride must be validated before entering render-stage.
- Decoder is skipped; pipeline receives validated, owner-annotated buffers.
- Raw ingress still must obey ownership and slab return semantics.

Use cases: upstream producers (camera, pre-decode pipelines) requiring strict low-latency render-only paths.

## 8. Pipeline Registration & Codegen Policy

- Pipeline registration is explicit and C-first; hand-written `src/pipeline/*.c` registration is preferred.
- Codegen may be used but must emit reviewed C committed to the repo or gated behind an opt-in CMake flag.
- The runtime must never parse DSL files at startup; DSL is a build-time convenience only.

Guidance and files of interest: [include/pipeline/job_presets.h](include/pipeline/job_presets.h), [include/pipeline/job_templates.h](include/pipeline/job_templates.h), [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L1)

## 9. SIMD Strategy & Execution

- Use SoA memory layout for image channels to maximize SIMD throughput.
- AVX2 and AVX-512 fused kernels are the production execution path for resize, color, and fused operations.
- Assembly limited to validated leaf kernels (rep movsb, rep stosb) and must not contain policy or allocation.

Kernel ABI and placement examples: [src/arch/x86_64/avx2/resize_avx2.c](src/arch/x86_64/avx2/resize_avx2.c#L1), [src/arch/x86_64/avx512/avx512.c](src/arch/x86_64/avx512/avx512.c#L1)

## 10. Memory Model & Locality

```
GLOBAL NUMA POOL
 ↓
thread-local slab
 ↓
thread-local arena
 ↓
decoded / prepared buffers
```

- NUMA-aware allocations and thread affinity to avoid cross-socket traffic.
- Slab allocator for hot buffers and O(1) allocation semantics.
- 64-byte alignment and prefetch hints to avoid false sharing and improve cache-line access.

See: [src/memory/slab_create.c](src/memory/slab_create.c#L1), [src/memory/numa.c](src/memory/numa.c#L1)

## 11. Runtime & Scheduling

- Worker-per-core model with pinned threads and SPSC queues per worker.
- Global MPMC submit path for API submissions; workers use `img_scheduler_steal` for load balancing.
- Backpressure and batching to honor latency and throughput budgets.

See: [src/runtime/scheduler.c](src/runtime/scheduler.c#L1), [src/runtime/queue_spsc.c](src/runtime/queue_spsc.c#L1)

## 12. Latency Budget & Performance Targets

| Stage    | Time   |
| -------- | ------ |
| Decode   | 0.5 ms |
| Dispatch | 0.2 ms |
| SIMD     | 0.8 ms |
| Output   | 0.3 ms |

Total target: < 2 ms (render-only paths)

## 13. Failure Strategy

- Corrupt input: fail-fast and return a clear error code.
- SIMD unsupported: fallback to scalar implementation.
- Memory full: reject job with resource error.
- Plugin failure: log and skip plugin stage if safe, otherwise fail job.

## 14. CI/CD & Enforcement

Gated checks (CI must fail the build if any of the following occur):

- `malloc` in pipeline hot-path (detected via static analysis or build-time instrumentation).
- Latency regression > 2ms on critical microbenchmarks.
- Throughput drop > 5% for core kernels.
- Undefined behavior detected by sanitizers (UBSan/ASan) in PR builds.

Add microbench thresholds and an `exported-symbols` ABI check to prevent accidental changes to public API.

## 15. Observability & Debugging

- Lightweight, sampled hooks for metrics/tracing; heavy tracing and logging only on cold-path or sampling windows.
- Binlog format for post-mortem analysis and deterministic replay.

## 16. Security

- Input validation is mandatory before ownership transfer; bounds checking must be enforced in cold-path.
- Sandbox integration (seccomp) is opt-in but recommended for untrusted inputs.

## 17. Implementation Roadmap (short-term)

- Finalize template registry APIs and strong ownership semantics (`img_template_entry_t`).
- Make `img_jump_table_init` and hardware registration deterministic and part of `img_api_init`.
- Add render-only benchmarks and CI thresholds.

## 18. Production Checklist (PR review)

- Build `IMGENGINE_SANITIZE` and run unit tests.
- Verify no heap allocations in hot path.
- Add microbench for changed kernels and include regression checks.
- Ensure generated C (if used) is checked into the repo or gated by opt-in CMake flag.

---

This HLD is intentionally prescriptive — the rules and policies here are the contract that implementation and reviewers must follow to reach deterministic, kernel-grade behavior.
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


