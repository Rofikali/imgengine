### RFC: imgengine v1.0 (FINAL)

    Title: High-Performance, Kernel-Grade Image Processing Engine
    Author: Principal Engineer (L7)
    Status: Final Draft
    Date: 2026-03-30

## 📁 FOLDER STRUCTURE

    imgengine/
    │
    ├── api/                              # 🔥 STABLE ABI (DO NOT BREAK)
    │   └── v1/
    │       ├── img_api.h
    │       ├── img_pipeline.h
    │       ├── img_buffer_utils.h
    │       ├── img_plugin_api.h
    │       └── img_error.h
    │
    ├── core/                             # 🔥 PURE COMPUTE (NO IO / THREADS)
    │   ├── context/
    │   │   ├── ctx.c
    │   │   ├── ctx.h
    │   │   └── ctx_internal.h
    │   │
    │   ├── buffer/
    │   │   ├── buffer.h
    │   │   ├── buffer_lifecycle.c
    │   │   └── buffer_lifecycle.h
    │   │
    │   ├── pipeline/
    │   │   ├── pipeline_desc.c
    │   │   ├── pipeline_compile.c
    │   │   ├── pipeline_types.h
    │   │   └── opcodes.h
    │   │
    │   ├── engine/
    │   │   ├── engine.c
    │   │   └── engine.h
    │   │
    │   └── config/
    │       ├── config.h
    │       └── config.c
    │
    ├── pipeline/                         # 🔥 HOT EXECUTION ONLY
    │   ├── exec/
    │   │   ├── pipeline_exec.c
    │   │   └── pipeline_exec.h
    │   │
    │   ├── batch/
    │   │   ├── batch_exec.c
    │   │   ├── batch_builder.c
    │   │   └── batch.h
    │   │
    │   ├── threaded/
    │   │   ├── pipeline_threaded.c
    │   │   └── pipeline_threaded.h
    │   │
    │   └── dispatch/
    │       ├── jump_table.c
    │       └── jump_table.h
    │
    ├── runtime/                          # 🔥 EXECUTION + SCHEDULING
    │   ├── worker/
    │   │   ├── worker.c
    │   │   └── worker.h
    │   │
    │   ├── scheduler/
    │   │   ├── scheduler.c
    │   │   └── scheduler.h
    │   │
    │   ├── queue/
    │   │   ├── mpmc.c
    │   │   ├── mpmc.h
    │   │   ├── spsc.c
    │   │   └── spsc.h
    │   │
    │   ├── dispatch/
    │   │   ├── exec_router.c
    │   │   └── exec_router.h
    │   │
    │   ├── affinity/
    │   │   ├── affinity.c
    │   │   └── affinity.h
    │   │
    │   └── cluster/                      # 🔥 (moved here)
    │       ├── cluster_registry.c
    │       └── cluster_registry.h
    │
    ├── memory/                           # 🔥 MEMORY SUBSYSTEM
    │   ├── slab/
    │   │   ├── slab.c
    │   │   ├── slab.h
    │   │   └── slab_internal.h
    │   │
    │   ├── arena/
    │   │   ├── arena.c
    │   │   └── arena.h
    │   │
    │   ├── numa/
    │   │   ├── numa.c
    │   │   └── numa.h
    │   │
    │   └── hugepage/
    │       ├── hugepage.c
    │       └── hugepage.h
    │
    ├── io/                               # 🔥 ALL IO (ISOLATED)
    │   ├── decoder/
    │   │   ├── decoder_entry.c
    │   │   └── streaming_decoder.c
    │   │
    │   ├── encoder/
    │   │   └── encoder_entry.c
    │   │
    │   ├── vfs/
    │   │   ├── memory_stream.c
    │   │   └── http_stream.c
    │   │
    │   └── remote/
    │       └── remote_fetch.c
    │
    ├── plugins/                          # 🔥 EXTENSIBILITY
    │   ├── builtin/
    │   │   ├── plugin_resize.c
    │   │   ├── plugin_crop.c
    │   │   ├── plugin_grayscale.c
    │   │   └── plugin_registry.c
    │   │
    │   └── dynamic/
    │       └── plugin_loader.c
    │
    ├── observability/                    # 🔥 YOUR STRONGEST PART (NOW PERFECT)
    │   ├── binlog/
    │   │   ├── binlog.c
    │   │   ├── binlog.h
    │   │   └── binlog_fast.h
    │   │
    │   ├── tracing/
    │   │   ├── tracing.c
    │   │   ├── tracing.h
    │   │   └── tracepoints.h
    │   │
    │   ├── metrics/
    │   │   ├── metrics.c
    │   │   └── metrics.h
    │   │
    │   ├── logger/
    │   │   ├── logger.c
    │   │   └── logger.h
    │   │
    │   ├── profiler/
    │   │   ├── profiler.c
    │   │   └── profiler.h
    │   │
    │   └── events/
    │       └── events.h
    │
    ├── security/                         # 🔥 HARDENING LAYER
    │   ├── sandbox/
    │   │   ├── sandbox.c
    │   │   └── sandbox.h
    │   │
    │   ├── validation/
    │   │   ├── input_validator.c
    │   │   └── input_validator.h
    │   │
    │   ├── bounds/
    │   │   └── bounds_check.h
    │   │
    │   ├── poison/
    │   │   └── poison.h
    │   │
    │   └── fuzz/
    │       └── fuzz_hooks.c
    │
    ├── arch/                             # 🔥 HARDWARE BACKENDS
    │   ├── x86/
    │   │   ├── avx2/
    │   │   └── avx512/
    │   │
    │   ├── arm/
    │   │   └── neon/
    │   │
    │   └── cpu_caps.c
    │
    ├── cold/                             # 🔥 NON-HOT UTILITIES
    │   ├── debug/
    │   │   ├── debug.c
    │   │   └── debug.h
    │   │
    │   ├── error/
    │   │   ├── error.c
    │   │   └── error.h
    │   │
    │   └── validation/
    │       ├── validation.c
    │       └── validation.h
    │
    ├── cmd/                              # 🔥 CLI + BENCH
    │   ├── imgengine/
    │   │   ├── main.c
    │   │   ├── args.c
    │   │   └── io_uring_engine.c
    │   │
    │   └── bench/
    │       └── lat_bench.c
    │
    ├── tests/                            # 🔥 REQUIRED FOR L10
    │   ├── unit/
    │   ├── fuzz/
    │   └── perf/
    │
    ├── build/
    ├── CMakeLists.txt
    └── README.md
