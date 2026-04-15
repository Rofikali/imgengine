imgengine L10++ Architecture Analyzer
============================================================

📡 Scanning source files...
✅ 153 functions | 1677 edges | 0.1s

HOT FUNCTIONS (your code only)
Function                                  Score  Fan-in  Fan-out
-----------------------------------------------------------------

  load_file_mmap                           36.5       8       57  🔥 HOT
  is_pdf_output                            36.5       8       57  🔥 HOT
  img_submit_task                          36.5       8       57  🔥 HOT
  img_encoded_free                         36.5       8       57  🔥 HOT
  img_api_shutdown                         36.5       8       57  🔥 HOT
  img_api_run_job                          36.5       8       57  🔥 HOT
  img_api_process_raw                      36.5       8       57  🔥 HOT
  img_api_process_fast                     36.5       8       57  🔥 HOT
  decode_image_secure                      36.5       8       57  🔥 HOT
  img_slab_free                            27.5      21       13  🔥 HOT
  img_slab_block_size                      20.5      14       13  🔥 HOT
  img_layout_grid                          20.5      12       17  🔥 HOT
  img_worker_stop                          20.0      14       12  🔥 HOT
  img_worker_join                          20.0      14       12  🔥 HOT
  img_worker_init                          20.0      14       12  🔥 HOT
  img_jump_table_init                      20.0      11       18  🔥 HOT
  img_slab_destroy                         19.5      13       13  🔥 HOT
  img_mpmc_push                            19.0      15        8  🔥 HOT
  img_mpmc_init                            19.0      15        8  🔥 HOT
  img_pipeline_execute_hot                 18.5      13       11  🔥 HOT

REAL BOTTLENECKS (your functions called by ≥3 callers)
  img_slab_free ← 21 callers
      ← align64
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 16 more
  img_mpmc_push ← 15 callers
      ← align_pow2
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 10 more
  img_mpmc_init ← 15 callers
      ← align_pow2
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 10 more
  img_worker_stop ← 14 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 9 more
  img_worker_join ← 14 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 9 more
  img_worker_init ← 14 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 9 more
  img_slab_block_size ← 14 callers
      ← align64
      ← img_buffer_from_slab
      ← img_canvas_free
      ← img_canvas_init
      ← img_decode_stb
      ... and 9 more
  img_slab_destroy ← 13 callers
      ← align64
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 8 more
  img_security_validate_request ← 13 callers
      ← LLVMFuzzerTestOneInput
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 8 more
  img_pipeline_execute_hot ← 13 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 8 more
  img_plugins_init_all ← 12 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 7 more
  img_layout_grid ← 12 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 7 more
  img_hw_register_kernels ← 12 callers
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ← img_api_shutdown
      ... and 7 more
  img_draw_crop_marks ← 12 callers
      ← decode_image_secure
      ← draw_h
      ← draw_v
      ← img_api_process_fast
      ← img_api_process_raw
      ... and 7 more
  img_arena_destroy ← 12 callers
      ← align64
      ← decode_image_secure
      ← img_api_process_fast
      ← img_api_process_raw
      ← img_api_run_job
      ... and 7 more

COMPLEX FUNCTIONS (fan-out ≥ 5 — consider splitting)
  load_file_mmap → 57 calls | src/api/api.c
  is_pdf_output → 57 calls | src/api/api.c
  img_submit_task → 57 calls | src/api/api.c
  img_encoded_free → 57 calls | src/api/api.c
  img_api_shutdown → 57 calls | src/api/api.c
  img_api_run_job → 57 calls | src/api/api.c
  img_api_process_raw → 57 calls | src/api/api.c
  img_api_process_fast → 57 calls | src/api/api.c
  decode_image_secure → 57 calls | src/api/api.c
  align_pow2 → 21 calls | src/runtime/queue_mpmc.c

LAYER VIOLATIONS (upward dependencies)
  ❌ ./include/pipeline/batch_exec.h includes api/v1/img_types.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/kernel_adapter.h includes api/v1/img_plugin_api.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/kernel_adapter.h includes api/v1/img_plugin_api.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/canvas.h includes api/v1/img_job.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/border.h includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/layout.h includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./include/pipeline/layout.h includes api/v1/img_job.h (pipeline[4] → api[8])
  ❌ ./include/security/input_validator.h includes api/v1/img_error.h (security[2] → api[8])
  ❌ ./include/core/context_internal.h includes arch/cpu_caps.h (core[0] → arch[3])
  ❌ ./include/core/context_internal.h includes runtime/worker.h (core[0] → runtime[5])
  ❌ ./src/plugins/plugin_grayscale.c includes api/v1/img_plugin_api.h (plugins[6] → api[8])
  ❌ ./src/plugins/plugin_registry.c includes api/v1/img_plugin_api.h (plugins[6] → api[8])
  ❌ ./src/plugins/plugin_resize.c includes api/v1/img_plugin_api.h (plugins[6] → api[8])
  ❌ ./src/memory/numa.c includes security/poision.h (memory[1] → security[2])
  ❌ ./src/memory/arena.c includes security/poision.h (memory[1] → security[2])
  ❌ ./src/memory/slab.c includes security/poision.h (memory[1] → security[2])
  ❌ ./src/pipeline/engine.c includes runtime/exec_router.h (pipeline[4] → runtime[5])
  ❌ ./src/pipeline/layout.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/layout.c includes api/v1/img_job.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/layout.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/batch_exec.c includes observability/profiler.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/logger.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/binlog_fast.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/tracepoints.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/profiler.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/logger.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/binlog_fast.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/batch_exec.c includes observability/tracepoints.h (pipeline[4] → observability[7])
  ❌ ./src/pipeline/bleed.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/canvas.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/border.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/pipeline/crop_marks.c includes api/v1/img_error.h (pipeline[4] → api[8])
  ❌ ./src/arch/x86_64/scalar/resize_scalar.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/scalar/resize_scalar.c includes pipeline/kernel_adapter.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx512/avx512.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/avx512/avx512.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/fused_resize_color_norm_avx2.c includes pipeline/batch.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/fused_resize_color_norm_avx2.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/resize_fused_avx2.c includes pipeline/batch.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/resize_fused_avx2.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/avx2/resize_fused_avx2.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/resize_avx2.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/avx2/resize_avx2.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/resize_h_avx2.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/avx2/resize_h_avx2.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/arch/x86_64/avx2/resize_v_avx2.c includes plugins/plugin_resize.h (arch[3] → plugins[6])
  ❌ ./src/arch/x86_64/avx2/resize_v_avx2.c includes pipeline/pipeline_types.h (arch[3] → pipeline[4])
  ❌ ./src/api/api.c includes io/decoder/decoder_entry.h (api[8] → io[9])
  ❌ ./src/api/api.c includes io/encoder/encoder_entry.h (api[8] → io[9])
  ❌ ./src/api/api.c includes io/decoder/decoder_entry.h (api[8] → io[9])
  ❌ ./src/api/api.c includes io/encoder/encoder_entry.h (api[8] → io[9])
  ❌ ./src/api/api.c includes io/encoder/pdf_encoder.h (api[8] → io[9])
  ❌ ./src/core/img_pipeline.c includes api/v1/img_pipeline.h (core[0] → api[8])
  ❌ ./src/core/img_pipeline.c includes pipeline/pipeline_types.h (core[0] → pipeline[4])
  ❌ ./src/core/img_pipeline.c includes pipeline/pipeline_compiled.h (core[0] → pipeline[4])
  ❌ ./src/core/init.c includes arch/cpu_caps.h (core[0] → arch[3])
  ❌ ./src/core/init.c includes pipeline/jump_table.h (core[0] → pipeline[4])
  ❌ ./src/core/init.c includes runtime/plugin_loader.h (core[0] → runtime[5])
  ❌ ./src/core/init.c includes security/sandbox.h (core[0] → security[2])
  ❌ ./src/core/context.c includes memory/slab.h (core[0] → memory[1])
  ❌ ./src/core/context.c includes memory/arena.h (core[0] → memory[1])
  ❌ ./src/core/batch_builder.c includes pipeline/batch_exec.h (core[0] → pipeline[4])
  ❌ ./src/core/batch_builder.c includes memory/arena.h (core[0] → memory[1])
  ❌ ./src/core/buffer_lifecycle.c includes runtime/buffer_lifecycle.h (core[0] → runtime[5])
  ❌ ./src/runtime/worker.c includes observability/binlog.h (runtime[5] → observability[7])
  ❌ ./src/runtime/rpc_server.c includes api/v1/img_buffer_utils.h (runtime[5] → api[8])
  ❌ ./src/runtime/plugin_loader.c includes api/v1/img_plugin_api.h (runtime[5] → api[8])

MODULE HEAT (which layers need attention)
Layer                 Functions  Total Score  Avg Score
--------------------------------------------------------

  api                         9        328.5       36.5  ████████████████████
  pipeline                   27        282.5       10.5  ████████████████████
  runtime                    21        213.0       10.1  ████████████████████
  observability              17        147.5        8.7  █████████████████
  memory                      8        123.0       15.4  ████████████████████
  arch                       18         82.0        4.6  █████████
  io                         14         76.0        5.4  ██████████
  cmd                         7         46.0        6.6  █████████████
  security                    4         38.0        9.5  ███████████████████
  core                        8         37.0        4.6  █████████
  plugins                     4         17.0        4.2  ████████

FILES TO SPLIT (too many dependencies)
  ❌ src/api/api.c → 20 deps → consider splitting
  ❌ src/pipeline/batch_exec.c → 9 deps → consider splitting
  ❌ src/io/decoder/streaming_decoder.c → 8 deps → consider splitting
  ❌ src/pipeline/layout.c → 7 deps → consider splitting

POTENTIALLY UNUSED FUNCTIONS (fan-in = 0)
  (may be exported symbols or entry points — verify manually)
  LLVMFuzzerTestOneInput  [src/security/fuzz_hooks.c]
  decode  [src/tools/binlog_decode.c]
  grayscale_exec  [src/plugins/plugin_grayscale.c]
  img_arch_grayscale_avx2  [src/arch/x86_64/avx2/color_avx2.c]
  img_arch_grayscale_scalar  [src/arch/x86_64/scalar/color_scalar.c]
  img_arch_resize_h_avx2  [src/arch/x86_64/avx2/resize_h_avx2.c]
  img_arch_resize_v_avx2  [src/arch/x86_64/avx2/resize_v_avx2.c]
  img_batch_execute_fused  [src/pipeline/batch_exec.c]
  img_batch_execute_rt  [src/hot/batch_exec.c]
  img_buffer_from_slab  [api/v1/img_buffer_utils.h]
  img_build_job  [src/cmd/imgengine/job_builder.c]
  img_cluster_register  [src/runtime/cluster_registry.c]
  img_ctx_destroy  [src/core/context.c]
  img_decode_stb  [src/io/decoder/stb_bridge.c]
  img_decode_stream  [src/io/decoder/streaming_decoder.c]
  img_encode_jpeg  [src/io/encoder/jpeg_encoder.c]
  img_engine_init  [src/core/init.c]
  img_exec_op_inline  [include/hot/pipeline_exec_inline.h]
  img_fused_init  [src/pipeline/fused_registry.c]
  img_log_error  [src/cold/error.c]

============================================================
WHAT TO DO NEXT — PRIORITY ORDER
============================================================

  [1] CRITICAL
      Fix 67 layer violation(s) — upward dependencies break module boundaries

  [2] HIGH
      Split img_submit_task() — 57 outgoing calls — in src/api/api.c

  [2] HIGH
      Split is_pdf_output() — 57 outgoing calls — in src/api/api.c

  [2] HIGH
      Split load_file_mmap() — 57 outgoing calls — in src/api/api.c

  [3] HIGH
      Replace resize_nearest() with bilinear AVX2 (src/pipeline/layout.c) — current is nearest-neighbour

  [3] HIGH
      Wire io_uring output path — currently main.c initializes io_uring but img_api_run_job() writes via fwrite(), not io_uring

  [4] MEDIUM
      Add --mode fit|fill CLI flag to expose IMG_FIT/IMG_FILL (job->mode is set but not wired to CLI args)

  [4] MEDIUM
      Add img_api_run_job_raw() — returns encoded bytes instead of writing to file; enables io_uring output and Python/Go bindings

  [5] LOW
      Implement PDF multi-page: current pdf_encoder.c = single page; for large grids that exceed A4, add page continuation

  [5] MEDIUM
      Add CI performance gate: bench_lat P99 > 2ms = build fail (RFC §22.3 requirement)

  [6] LOW
      Add Python CFFI bindings (bindings/python/cffi_wrapper.py) — expose img_api_init, img_api_run_job, img_api_shutdown

============================================================

🚨 VIOLATIONS FOUND — fix before merging

### VIOLATION COUNT AFTER FIXES

    /*
    BEFORE: 67 violations
    AFTER applying all 6 steps:
    Group A (core → everything):    ~15 violations → 0  [startup/ absorbs init.c]
    Group B (arch → plugins/pipeline): 8 violations → 0  [resize_params.h in arch/]
    Group C (pipeline → api):         ~10 violations → 0  [result.h in core/, job.h in pipeline/]
    Group D (memory → security):        3 violations → 0  [poison.h in memory/]
    Group E (api → io):                 3 violations → 0  [io_vtable.h]
    Group F (pipeline → observability): 8 violations → kept (justified — see note)

    Note on Group F (pipeline/batch_exec.c → observability/):
    The observability dependency in batch_exec.c is legitimate —
    the hot path batch executor records latency metrics.
    Fix: make observability lower in the DAG (move to index 3, below pipeline).
    Or: use a lock-free macro that compiles to nothing without observability.
    This is the Linux kernel approach: TRACE_EVENT macros are no-ops when
    tracing is disabled. Add IMGENGINE_OBSERVABILITY flag to CMakeLists.

    FINAL EXPECTED: ~0-8 violations (only the observability/pipeline edge remains)
    */
