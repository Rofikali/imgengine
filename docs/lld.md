# Low-Level Design (LLD) — imgengine

## Purpose

This document maps the major headers in `include/` to their implementing sources in `src/`, outlines the main call flows, and highlights key files to inspect when working on a subsystem.


## How this LLD was produced

This document is an expanded, production-ready mapping derived from the v2.0 RFC and the current repository layout. It focuses on concrete data structures, call-sites, and implementation points required to implement the HLD rules in code.

## Navigation cheat-sheet (where to look)

- Public API surface: `include/api/v1` → implementations under `src/api/` (e.g. [src/api/api_init.c](src/api/api_init.c#L1)).
- Core/context & buffers: `include/core/*` → `src/core/*` (e.g. `include/core/buffer.h` → [src/core/buffer_lifecycle.c](src/core/buffer_lifecycle.c#L1)).
- Pipeline engine: `include/pipeline/*` → `src/pipeline/*` (jump-table, fused kernels, pipeline executor).
- IO layer: `include/io/*` → `src/io/decoder/*`, `src/io/encoder/*`, `src/io/vfs/*`.
- Memory allocators: `include/memory/*` → `src/memory/*` (slab, arena, numa, hugepage).
- Runtime & scheduler: `include/runtime/*` → `src/runtime/*` (scheduler, queues, worker loop).
- Arch-specific kernels: `include/arch/*` → `src/arch/<arch>/*` (AVX2/AVX512/Scalar/NEON implementations).

## Representative mappings and key symbols

- `img_api_init` — [src/api/api_init.c](src/api/api_init.c#L9)
- `img_api_shutdown` — [src/api/api_shutdown.c](src/api/api_shutdown.c#L6)
- `img_api_run_job_impl` — [src/api/api_job_run.c](src/api/api_job_run.c#L99)
- `g_jump_table` — [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L4)
- `img_register_op` — [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L7)
- `img_slab_alloc` / `img_slab_free` — [src/memory/slab_hot.c](src/memory/slab_hot.c#L1)

## Data structures (conceptual LLD)

### Prepared template registry

Concept:

```
typedef struct {
	img_job_template_t template_id; // stable id
	img_job_t job;                  // fully populated job template
	uint32_t caps;                  // capability bits (SIMD, alignments)
} img_template_entry_t;

// runtime registry (constant-time lookup)
img_template_entry_t *registry; // allocated at startup, indexed by TemplateKey
```

Responsibilities:

- constant-time lookup during API handling
- pre-populated job templates with safe defaults
- thread-local overrides applied at submit time

Relevant files: [src/pipeline/fused_registry.c](src/pipeline/fused_registry.c#L1)

### Prepared decoder

Concept:

```
typedef struct {
	img_engine_t *engine;
	img_ctx_t ctx; // bound API context
	// preallocated thread-local decode state (slab + arena)
} img_prepared_decoder_t;
```

APIs:

- `img_api_prepare_decoder(engine, &decoder)` — prepare decoder state for a thread
- `img_api_decode_prepared(&decoder, input, size, &out)` — decode using prepared state

Relevant files: [src/api/api_process_fast_decode.c](src/api/api_process_fast_decode.c#L1)

### Jump-table and Dispatch

Conceptual hot-path:

```
extern void (*g_jump_table[])(img_ctx_t*, void*, const void* params);

// Dispatch inside pipeline hot path
g_jump_table[op_code](ctx, buffer, params);
```

Properties:

- O(1) dispatch
- branch-free selection when combined with preselected jump-table index
- jump-table populated at deterministic startup via `img_register_op`

Files: [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L1), [src/pipeline/jump_table_init.c](src/pipeline/jump_table_init.c#L1)

### SPSC queue (hot path messaging)

Key properties: lock-free, cache-line aligned atomics, ABA avoidance via single-producer/single-consumer contract.

Minimal sketch (LLD):

```
typedef struct {
	alignas(64) atomic_uint head;
	char pad1[64];
	alignas(64) atomic_uint tail;
	char pad2[64];
	void* slots[QUEUE_SIZE];
} spsc_queue_t;
```

Files: [src/runtime/queue_spsc.c](src/runtime/queue_spsc.c#L1)

### Slab allocator (hot path)

Conceptual API:

```
img_slab_t *img_slab_create(size_t block_size, size_t nblocks);
void *img_slab_alloc(img_slab_t *s);
void img_slab_recycle(img_slab_t *s, void *ptr);
```

Properties:

- O(1) allocation from a preallocated pool
- No locking when used thread-locally
- Owned by a thread-local arena or global NUMA pool

Files: [src/memory/slab_create.c](src/memory/slab_create.c#L1), [src/memory/slab_hot.c](src/memory/slab_hot.c#L1)

## Call flows (detailed)

### CLI submit → run (render-only)

1. `src/cmd/imgengine/job_builder.c` builds a `img_job_t` from CLI args.
2. `src/api/api_job_run.c` resolves TemplateKey via the prepared registry.
3. `src/runtime/scheduler_submit.c` enqueues the prepared `img_job_t` into a per-worker SPSC or global MPMC queue.
4. Worker thread in `src/runtime/worker.c` dequeues and constructs ephemeral `img_ctx_t` and execution batch.
5. `src/pipeline/pipeline_executor.c`/`src/hot/pipeline_exec.c` execute fused pipeline using `g_jump_table` dispatch into `src/arch/*` kernels.
6. Output is encoded via `src/io/encoder/*` and finalized in `src/api/api_job_finish_output.c`.

### Prepared decode path

1. A prepared decoder is created at API or startup time (`img_api_prepare_decoder`).
2. Incoming bytes are passed to `img_api_decode_prepared` which uses thread-local slab buffers and returns ownership to caller.
3. Ownership contracts ensure decoded buffers are recycled back to the same slab pool after pipeline execution.

## Testing & CI LLD guidance

- Add unit tests for `img_slab_*` (allocate/recycle) and SPSC/MPMC queue semantics.
- Add microbench harness per-kernel (AVX2/AVX-512/Scalar) with regression thresholds tracked in CI.
- Add a tool to compare current exported symbols against `docs/abi/exported_symbols.json` and fail CI on unexpected ABI changes.

## LLD Action Items (concrete)

1. Implement a deterministic `img_jump_table_init` ordering and record it in startup logs.
2. Harden the prepared template registry with clear ownership and thread-safety invariants.
3. Add regression microbenchmarks for `resize_avx2`, `resize_avx512` and fused kernels.
4. Provide a `sanitizer` build (ASan/UBSan) for CI and verify fallbacks to scalar kernels succeed.

---

This LLD is intended to be the canonical mapping engineers use when authoring kernel changes, adding registers, or implementing new pipelines. It focuses on deterministic initialization, ownership semantics, and small, testable hot-path primitives.
