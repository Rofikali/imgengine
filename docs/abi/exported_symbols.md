# Exported Symbols — Public ABI Reference

This file lists the canonical exported symbols that form the public runtime ABI. CI should compare the live binary exports against this file (or `docs/abi/exported_symbols.json`) to detect accidental ABI drift.

| Symbol | Location |
|--------|----------|
| `img_api_init` | [src/api/api_init.c](src/api/api_init.c#L9) |
| `img_api_shutdown` | [src/api/api_shutdown.c](src/api/api_shutdown.c#L6) |
| `img_api_run_job_impl` | [src/api/api_job_run.c](src/api/api_job_run.c#L99) |
| `img_api_run_job_raw_impl` | [src/api/api_job_run_raw.c](src/api/api_job_run_raw.c#L4) |
| `img_api_run_job_rgb24_impl` | [src/api/api_job_run_rgb24.c](src/api/api_job_run_rgb24.c#L33) |
| `img_api_run_job_rgb24_raw_impl` | [src/api/api_job_run_rgb24.c](src/api/api_job_run_rgb24.c#L84) |
| `img_api_process_raw` | [src/api/api_process_raw.c](src/api/api_process_raw.c#L7) |
| `img_api_process_fast` | [src/api/api_process_fast.c](src/api/api_process_fast.c#L5) |
| `img_api_release_raw_buffer` | [src/api/api_encoded.c](src/api/api_encoded.c#L8) |
| `img_encoded_free` | [src/api/api_encoded.c](src/api/api_encoded.c#L13) |
| `img_pipeline_create` | [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L27) |
| `img_pipeline_destroy` | [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L37) |
| `img_pipeline_add_op` | [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L42) |
| `g_jump_table` | [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L4) |
| `g_batch_jump_table` | [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L5) |
| `img_register_op` | [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L7) |
| `img_jump_table_init` | [src/pipeline/jump_table_init.c](src/pipeline/jump_table_init.c#L5) |
| `img_scheduler_init` | [src/runtime/scheduler_init.c](src/runtime/scheduler_init.c#L18) |
| `img_scheduler_destroy` | [src/runtime/scheduler_destroy.c](src/runtime/scheduler_destroy.c#L8) |
| `img_scheduler_submit` | [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L8) |
| `img_runtime_submit_task` | [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L22) |
| `img_runtime_wait_task` | [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L30) |
| `img_scheduler_steal` | [src/runtime/scheduler.c](src/runtime/scheduler.c#L5) |
| `img_slab_create` | [src/memory/slab_create.c](src/memory/slab_create.c#L9) |
| `img_slab_destroy` | [src/memory/slab_destroy.c](src/memory/slab_destroy.c#L8) |
| `img_slab_alloc` | [src/memory/slab_hot.c](src/memory/slab_hot.c#L6) |
| `img_slab_free` | [src/memory/slab_hot.c](src/memory/slab_hot.c#L19) |
| `img_slab_recycle` | [src/memory/slab_hot.c](src/memory/slab_hot.c#L31) |
| `img_slab_block_size` | [src/memory/slab_block_size.c](src/memory/slab_block_size.c#L5) |
| `img_arch_avx2_resize` | [src/arch/x86_64/avx2/resize_avx2.c](src/arch/x86_64/avx2/resize_avx2.c#L129) |
| `resize_avx2` | [src/arch/x86_64/avx2/resize_avx2.c](src/arch/x86_64/avx2/resize_avx2.c#L211) |
| `img_arch_grayscale_avx2` | [src/arch/x86_64/avx2/color_avx2.c](src/arch/x86_64/avx2/color_avx2.c#L20) |
| `img_arch_grayscale_scalar` | [src/arch/x86_64/scalar/color_scalar.c](src/arch/x86_64/scalar/color_scalar.c#L6) |
| `g_io_vtable` | [src/io/io_register.c](src/io/io_register.c#L16) |
| `img_io_register` | [src/io/io_register.c](src/io/io_register.c#L18) |
| `img_logger_init` | [src/observability/logger_lifecycle.c](src/observability/logger_lifecycle.c#L7) |
| `img_logger_shutdown` | [src/observability/logger_lifecycle.c](src/observability/logger_lifecycle.c#L16) |
| `img_log_write` | [src/observability/logger_write.c](src/observability/logger_write.c#L8) |

---

Notes:

- Keep `docs/abi/exported_symbols.json` synchronized with this markdown table; use a CI task to fail on mismatches.
- When adding new public API symbols, update both JSON and MD files and document the ABI contract and expected lifetime.
# Exported Symbols — API / ABI Audit

Scope: public headers under `include/api/v1/` plus selected internal headers that form the stable/visible ABI (pipeline jump table, runtime scheduler, memory slab, arch interface, IO vtable, observability).

Generated: 2026-04-26

## Format
- Header: <path>
- Symbol: brief signature — Implementation: <src path>#Lline

---

## include/api/v1/img_api.h

- `img_engine_t` (opaque handle) — forward-declared in header; runtime structure in `src/core/context_internal.h` and `src/core/context.c`.
- `img_api_init(uint32_t workers)` — Implementation: [src/api/api_init.c](src/api/api_init.c#L9)
- `img_api_shutdown(img_engine_t *engine)` — Implementation: [src/api/api_shutdown.c](src/api/api_shutdown.c#L6)
- `img_api_run_job(...)` — Public wrapper in [src/api/api_job.c](src/api/api_job.c#L5) → core impl [src/api/api_job_run.c](src/api/api_job_run.c#L99)
- `img_api_run_job_raw(...)` — wrapper [src/api/api_job.c](src/api/api_job.c#L17) → impl [src/api/api_job_run_raw.c](src/api/api_job_run_raw.c#L4)
- `img_api_run_job_rgb24(...)` — wrapper [src/api/api_job.c](src/api/api_job.c#L34) → impl [src/api/api_job_run_rgb24.c](src/api/api_job_run_rgb24.c#L33)
- `img_api_run_job_rgb24_raw(...)` — wrapper [src/api/api_job.c](src/api/api_job.c#L42) → impl [src/api/api_job_run_rgb24.c](src/api/api_job_run_rgb24.c#L84)
- `img_api_process_raw(...)` — Implementation: [src/api/api_process_raw.c](src/api/api_process_raw.c#L7)
- `img_api_process_fast(...)` — Implementation: [src/api/api_process_fast.c](src/api/api_process_fast.c#L5)
- `img_api_release_raw_buffer(img_engine_t*, img_buffer_t*)` — Implementation: [src/api/api_encoded.c](src/api/api_encoded.c#L8)
- `img_encoded_free(uint8_t *ptr)` — Implementation: [src/api/api_encoded.c](src/api/api_encoded.c#L13)

Notes: many API functions are thin wrappers that forward to `_impl` variants (see `src/api/*_impl` files and `api_job_run.c` for job lifecycle).

## include/api/v1/img_pipeline.h

- `img_pipeline_create(void)` — Implementation: [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L27)
- `img_pipeline_destroy(img_pipeline_t *pipe)` — Implementation: [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L37)
- `img_pipeline_add_op(img_pipeline_t *pipe, uint32_t op_code, void *params)` — Implementation: [src/pipeline/pipeline_executor.c](src/pipeline/pipeline_executor.c#L42)

## include/pipeline/plugin_abi.h

- ABI types exported for plugins: `img_plugin_descriptor_t`, `img_plugin_getter_fn`, macros `IMG_PLUGIN_EXPORT_SYMBOL`, `IMG_REGISTER_PLUGIN` — Consumers implement `img_get_plugin_descriptor()` in `src/plugins/*` or external plugin objects; loader located at [src/runtime/plugin_loader.c](src/runtime/plugin_loader.c#L1).

## include/pipeline/jump_table.h

- `extern img_kernel_fn g_jump_table[IMG_MAX_OPS]` — defined/zero-initialized in [src/pipeline/jump_table.c](src/pipeline/jump_table.c#L21)
- `extern img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS]` — defined in [src/pipeline/jump_table.c](src/pipeline/jump_table.c#L21)
- `void img_register_op(uint32_t opcode, img_op_fn single_fn, img_batch_op_fn batch_fn)` — register helper: [src/pipeline/jump_table_register.c](src/pipeline/jump_table_register.c#L7)
- `void img_jump_table_init(cpu_caps_t caps)` — init (CPU dispatch): [src/pipeline/jump_table_init.c](src/pipeline/jump_table_init.c#L5)

Runtime wiring: hardware registration populates entries in [src/pipeline/hardware_registry.c](src/pipeline/hardware_registry.c#L9-L18).

## include/runtime/scheduler.h

- `int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)` — Implementation: [src/runtime/scheduler_init.c](src/runtime/scheduler_init.c#L18)
- `void img_scheduler_destroy(img_scheduler_t *sched)` — Implementation: [src/runtime/scheduler_destroy.c](src/runtime/scheduler_destroy.c#L8)
- `int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task)` — Implementation: [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L8)
- `int img_runtime_submit_task(img_engine_t *engine, img_task_t *task)` — Implementation: [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L22)
- `img_result_t img_runtime_wait_task(img_task_t *task)` — Implementation: [src/runtime/scheduler_submit.c](src/runtime/scheduler_submit.c#L30)
- `img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id)` — Implementation: [src/runtime/scheduler.c](src/runtime/scheduler.c#L5)

Worker loop: `src/runtime/worker_loop.c` polls per-worker SPSC, then `g_task_queue` (MPMC), then steal — see `img_worker_loop()`.

## include/memory/slab.h

- `img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size)` — Implementation: [src/memory/slab_create.c](src/memory/slab_create.c#L9)
- `void img_slab_destroy(img_slab_pool_t *pool)` — Implementation: [src/memory/slab_destroy.c](src/memory/slab_destroy.c#L8)
- `uint8_t *img_slab_alloc(img_slab_pool_t *pool)` — Implementation: [src/memory/slab_hot.c](src/memory/slab_hot.c#L6)
- `void img_slab_free(img_slab_pool_t *pool, void *ptr)` — Implementation: [src/memory/slab_hot.c](src/memory/slab_hot.c#L19)
- `void img_slab_recycle(img_slab_pool_t *pool, void *ptr)` — Implementation: [src/memory/slab_hot.c](src/memory/slab_hot.c#L31)
- `size_t img_slab_block_size(img_slab_pool_t *pool)` — Implementation: [src/memory/slab_block_size.c](src/memory/slab_block_size.c#L5)

## include/core/buffer.h

- `img_buf_header_t` and `img_buffer_t` types — defined in header; helpers:
  - `static inline img_buf_header_t *img_buf_hdr(img_buffer_t *buf)` — header in same file (inline)

## include/arch/arch_interface.h

- Unified kernel prototypes (ABI): `img_kernel_fn` and a set of arch-specific symbols. Representative implementations:
  - `img_arch_avx2_resize(...)` — [src/arch/x86_64/avx2/resize_avx2.c](src/arch/x86_64/avx2/resize_avx2.c#L129)
  - `resize_avx2(...)` wrapper — [src/arch/x86_64/avx2/resize_avx2.c](src/arch/x86_64/avx2/resize_avx2.c#L211)
  - `img_arch_grayscale_avx2(...)` — [src/arch/x86_64/avx2/color_avx2.c](src/arch/x86_64/avx2/color_avx2.c#L20)
  - `img_arch_grayscale_scalar(...)` — [src/arch/x86_64/scalar/color_scalar.c](src/arch/x86_64/scalar/color_scalar.c#L6)

Arch kernel implementations are registered into the jump table during startup (see `src/startup/engine_init_compute.c`).

## include/io/io_vtable.h

- `extern img_io_vtable_t g_io_vtable` — declaration; defined/initialized in [src/io/io_register.c](src/io/io_register.c#L16)
- `void img_io_register(img_decode_fn decode, img_encode_fn encode, img_encode_pdf_fn encode_pdf)` — Implementation: [src/io/io_register.c](src/io/io_register.c#L18)

## include/observability/logger.h

- `void img_logger_init(void)` — Implementation: [src/observability/logger_lifecycle.c](src/observability/logger_lifecycle.c#L7)
- `void img_logger_shutdown(void)` — Implementation: [src/observability/logger_lifecycle.c](src/observability/logger_lifecycle.c#L16)
- `void img_log_write(img_log_level_t level, const char *fmt, ...)` — Implementation: [src/observability/logger_write.c](src/observability/logger_write.c#L8)

---

If you want, I can produce:

- a machine-readable CSV/JSON of symbol → file:line for CI checks,
- an exported-symbols header-only text suitable for linker/ABI audits,
- per-object symbol lists by running `nm` on built objects (requires building the project).

Next step (pick one): CSV/JSON export, or generate a narrower ABI list for `include/api/v1/*` only? 
