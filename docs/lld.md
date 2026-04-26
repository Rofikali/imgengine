# Low-Level Design (LLD) — imgengine

## Purpose

This document maps the major headers in `include/` to their implementing sources in `src/`, outlines the main call flows, and highlights key files to inspect when working on a subsystem.

## How this LLD was produced

Scanned `include/` and `src/` directories to identify module boundaries and representative implementation files. This LLD is a starting point — function-level diagrams and call graphs can be added next.

## Navigation cheat-sheet (where to look)

- Public API surface: `include/api/v1` -> implementations under `src/api/` (e.g. `src/api/api.c`, `src/api/api_job_run.c`).
- Core/context & buffers: `include/core/*` -> `src/core/*` (e.g. `include/core/buffer.h` -> `src/core/buffer_lifecycle.c`).
- Pipeline engine: `include/pipeline/*` -> `src/pipeline/*` (jump_table, fused kernels, pipeline executor).
- IO layer: `include/io/*` -> `src/io/decoder/*`, `src/io/encoder/*`, `src/io/vfs/*`.
- Memory allocators: `include/memory/*` -> `src/memory/*` (slab, arena, numa, hugepage).
- Runtime & scheduler: `include/runtime/*` -> `src/runtime/*` (scheduler, queues, worker loop, RPC glue).
- Plugins: `include/plugins/*` and `include/pipeline/plugin_abi.h` -> `src/plugins/*` and `src/runtime/plugin_loader.c`.
- Arch-specific kernels: `include/arch/*` -> `src/arch/<arch>/*` (AVX2/AVX512/Scalar/NEON implementations).
- Observability: `include/observability/*` -> `src/observability/*` (logger, binlog, metrics, tracing).
- Security: `include/security/*` -> `src/security/*` (input validation, sandbox helpers).

## Representative file mappings (quick reference)

- API: `include/api/v1/img_api.h`, `include/api/v1/img_job.h`  ⇄ `src/api/api.c`, `src/api/api_job.c`, `src/api/api_job_run.c`
- Pipeline: `include/pipeline/pipeline.h`, `include/pipeline/pipeline_compiled.h` ⇄ `src/pipeline/pipeline_executor.c`, `src/pipeline/pipeline_build.c`, `src/pipeline/pipeline_fuse*.c`
- Hot execution: `include/hot/pipeline_exec.h` ⇄ `src/hot/pipeline_exec.c`, `src/hot/batch_exec.c`
- Memory: `include/memory/slab.h` ⇄ `src/memory/slab.c`, `src/memory/slab_create.c`, `src/memory/slab_lifecycle.c`
- IO: `include/io/decoder/decoder_entry.h` ⇄ `src/io/decoder/decoder_entry.c`, `src/io/encoder/encoder_entry.c`
- Runtime: `include/runtime/scheduler.h` ⇄ `src/runtime/scheduler.c`, `src/runtime/worker.c`, `src/runtime/queue_mpmc.c`
- Plugins: `include/pipeline/plugin_abi.h` ⇄ `src/runtime/plugin_loader.c`, `src/plugins/plugin_registry.c`

## Common call flow (detailed example)

CLI path (simplified):

1. `src/cmd/imgengine/main.c` parses args and constructs a job via `job_builder.c`.
2. Job is submitted to API layer (`src/api/api_task_submit.c`, `src/api/api_job_run.c`).
3. API enqueues work via `src/runtime/scheduler_submit.c` into runtime queues.
4. Worker thread (`src/runtime/worker.c`) dequeues a job and prepares a batch.
5. If pipeline is compiled/fused, `src/pipeline/pipeline_executor.c` / `src/hot/pipeline_exec.c` execute fused kernels.
6. Execution routes to arch-specific kernels under `src/arch/<arch>/` (e.g. `src/arch/x86_64/avx2/resize_avx2.c`).
7. Result buffers flow through encoder (`src/io/encoder/jpeg_encoder.c`) and job finalization (`src/api/api_job_finish_output.c`).

## Memory & Buffer lifecycle (notes)

- Buffers are allocated via arena/slab APIs (`include/memory/*` and `src/memory/*`).
- Buffer ownership and release are tracked in `include/core/buffer.h` and lifecycle helpers in `src/core/buffer_lifecycle.c` and runtime raw buffer release in `src/runtime/raw_buffer_release.c`.

## Plugin ABI and discovery

- ABI spec: `include/pipeline/plugin_abi.h` (defines expected plugin exports and version compatibility).
- Loader: `src/runtime/plugin_loader.c` (reads descriptors, validates ABI and registers plugins into `plugin_registry`).
- Registrations: `src/plugins/plugin_registry*.c` implement registry bookkeeping and validation.

## Jump-table, fused kernels and codegen

- `include/pipeline/jump_table.h` and `src/pipeline/jump_table*.c` implement function pointer selection for resize/fused ops.
- Fused kernel registry and generated kernels appear under `src/pipeline/fused_*` and `src/pipeline/generated.c` (the latter is produced by DSL codegen when enabled).

## Observability hooks

- Instrumentation points are sprinkled across `src/api`, `src/runtime` and `src/pipeline` using `include/observability/*` primitives (binlog, metrics and tracing). Logger lifecycle and async writer live in `src/observability/logger_*.c`.

## How to find the implementing file for a header / symbol

Use ripgrep (or grep) from the repo root:

```bash
# find headers quickly
find include -name '*.h' | sed -e 's:^:include/:g' | sort

# locate implementations for a header name (example: buffer.h)
rg "buffer_lifecycle|buffer_" src -n || rg "buffer_" src -S

# find where a public API function is defined
rg "\bIMG_\w+\b" src include || rg "img_job_" src -n
```

## Recommended next steps to expand LLD

- Generate function-level call graphs for critical flows (job run, pipeline execution) using cscope/clangd or callgraph extraction.
- Populate per-file responsibilities and list of exported symbols in `include/*` with direct links to implementation lines in `src/*`.
- Add sequence diagrams for the job lifecycle and plugin load lifecycle.

## Appendix — quick pointers

- Public API headers: `include/api/v1/*`
- CLI entry and builders: `src/cmd/imgengine/main.c`, `src/cmd/imgengine/job_builder.c`
- Scheduler & workers: `src/runtime/scheduler.c`, `src/runtime/worker.c`
- Hot execution: `src/hot/pipeline_exec.c`, `src/hot/batch_exec.c`
- Pipeline runtime: `src/pipeline/pipeline_executor.c`, `src/pipeline/pipeline_build.c`
- Encoders/Decoders: `src/io/encoder/*`, `src/io/decoder/*`

## Problem → Pattern Map (LLD)

This section maps concrete low-level files and hotspots to the design patterns you should consider when implementing fixes or new features.

- **Strategy** — multiple algorithms/kernels: inspect `src/pipeline/jump_table.c`, `src/pipeline/jump_table_select_resize_avx2.c`, `src/pipeline/hardware_registry.c`. How to apply: ensure `img_jump_table_init` selects best kernel and is invoked early in startup.
- **Factory** — complex object creation: inspect `src/pipeline/pipeline_executor.c` (`img_pipeline_create`) and `src/api/api_init.c` (`img_api_init`). How to apply: centralize allocation/config for pipelines and engine contexts.
- **Observer** — metrics/tracing: inspect `src/observability/logger_write.c`, `src/observability` hooks. How to apply: add sampled hooks into `src/runtime` and `src/pipeline` (low-overhead paths).
- **Decorator** — add behavior without changing kernel code: inspect `src/pipeline/generated.c` and `src/runtime/plugin_loader.c`. How to apply: implement thin wrapper functions that call kernels and perform pre/post actions.
- **Singleton** — global registries: `g_jump_table` (`src/pipeline/jump_table_register.c`), `g_io_vtable` (`src/io/io_register.c`). How to apply: keep init order strict and provide clear lifecycle APIs.
- **Proxy** — layered IO access control & caching: inspect `src/io/io_register.c` and `src/io/decoder/*`. How to apply: add a proxy layer for remote fetches and a small in-memory cache with size limits.
- **Pipeline / Chain-of-Responsibility** — flexible op ordering and batching: `src/pipeline/pipeline_executor.c`, `src/hot/pipeline_exec.c`. How to apply: represent ops as compact commands for reuse, batching and replay.
- **Data-Oriented Design (DOD)** — hot kernels like `src/arch/x86_64/avx2/resize_avx2.c` benefit from contiguous layouts and struct-of-arrays.
- **Lock-free messaging + work-stealing** — runtime core: `src/runtime/scheduler_submit.c`, `src/runtime/worker_loop.c`, `src/runtime/scheduler.c`.

## Bench findings → LLD action items

- **Decode hotspot (`src/io/decoder/*`)**: add a `Factory` + `Strategy` to choose streaming vs bulk decoder; consider prefetching and a decoder worker pool.
- **Encode hotspot (`src/io/encoder/*`)**: offload encode to a dedicated worker pool; instrument with microbenchmarks and thresholds in CI.
- **Throughput**: verify per-worker SPSC queues and `img_scheduler_steal` behavior under load; add targeted microbench for SPSC throughput and lock-free paths.

## Next LLD steps

- Implement decoder factory & simple strategy toggle; microbench it and commit results.
- Add a `Decorator` example wrapping one kernel (small patch) to demonstrate caching and validation without changing kernel implementations.
