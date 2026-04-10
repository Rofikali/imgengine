### IMGENGINE — MASTER FOLDER STRUCTURE (L10 CLEAN)

imgengine/
│
api/v1/                                 # 🔥 STABLE ABI (DO NOT BREAK)
    ├── img_types.h        # ONLY pure types (no logic)
    ├── img_api.h          # lifecycle + submission
    ├── img_pipeline.h     # pipeline construction
    ├── img_buffer.h       # opaque buffer API (NO internals)
    └── img_error.h        # error model
include/
    ├── core/                                 # 🔥 PURE COMPUTE (NO IO / THREADS)
    │   ├── context/
    │   │   ├── ctx.h
    │   │   └── ctx_internal.h
    │   │
    │   ├── buffer/
    │   │   ├── buffer.h
    │   │   └── buffer_lifecycle.h
    │   │
    │   ├── pipeline/
    │   │   ├── pipeline_types.h
    │   │   └── opcodes.h
    │   │
    │   ├── engine/
    │   │   └── engine.h
    │   │
    │   └── config/
    │       ├── config.h
    │
    ├── pipeline/                             # 🔥 HOT EXECUTION ONLY
    │   ├── exec/
    │   │   └── pipeline_exec.h
    │   │
    │   ├── batch/
    │   │   └── batch.h
    │   │
    │   ├── threaded/
    │   │   └── pipeline_threaded.h
    │   │
    │   └── dispatch/
    │       └── jump_table.h
    │
    ├── runtime/                              # 🔥 EXECUTION + SCHEDULING
    │   ├── worker/
    │   │   └── worker.h
    │   │
    │   ├── scheduler/
    │   │   └── scheduler.h
    │   │
    │   ├── queue/
    │   │   ├── mpmc.h
    │   │   └── spsc.h
    │   │
    │   ├── dispatch/
    │   │   └── exec_router.h
    │   │
    │   ├── affinity/
    │   │   └── affinity.h
    │   │
    │   └── cluster/                          # 🔥 CLUSTER RUNTIME
    │       └── cluster_registry.h
    │
    ├── memory/                               # 🔥 MEMORY SUBSYSTEM
    │   ├── slab/
    │   │   ├── slab.h
    │   │   └── slab_internal.h
    │   │
    │   ├── arena/
    │   │   └── arena.h
    │   │
    │   ├── numa/
    │   │   └── numa.h
    │   │
    │   └── hugepage/
    │       └── hugepage.h
    │
    ├── io/                                   # 🔥 ALL IO (STRICTLY ISOLATED)
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
    │
    ├── plugins/                              # 🔥 EXTENSIBILITY LAYER
    │   ├── builtin/
    │   │   ├── plugin_resize.c
    │   │   ├── plugin_crop.c
    │   │   ├── plugin_grayscale.c
    │   │   └── plugin_registry.c
    │   │
    │   └── dynamic/
    │       └── plugin_loader.c
    │
    ├── observability/                        # 🔥 PERFORMANCE + DEBUG POWER
    │   ├── binlog/
    │   │   ├── binlog.h
    │   │   └── binlog_fast.h
    │   │
    │   ├── tracing/
    │   │   ├── tracing.h
    │   │   └── tracepoints.h
    │   │
    │   ├── metrics/
    │   │   └── metrics.h
    │   │
    │   ├── logger/
    │   │   └── logger.h
    │   │
    │   ├── profiler/
    │   │   └── profiler.h
    │   │
    │   └── events/
    │       └── events.h
    │
    ├── security/                             # 🔥 HARDENING LAYER
    │   ├── sandbox/
    │   │   └── sandbox.h
    │   │
    │   ├── validation/
    │   │   └── input_validator.h
    │   │
    │   ├── bounds/
    │   │   └── bounds_check.h
    │   │
    │   ├── poison/
    │   │   └── poison.h
    │   │
    │   └── fuzz/
    │       └── fuzz_hooks.h
    │
    ├── arch/                                 # 🔥 HARDWARE BACKENDS
    │   ├── x86/
    │   │   ├── avx2/
    │   │   └── avx512/
    │   │
    │   ├── arm/
    │   │   └── neon/
    │   │
    │   └── cpu_caps.h
    │
    ├── cold/                                 # 🔥 NON-HOT UTILITIES
    │   ├── debug/
    │   │   └── debug.h
    │   │
    │   ├── error/
    │   │   └── error.h
    │   │
    │   └── validation/
    │       └── validation.h
src/
    ├── core/                                 # 🔥 PURE COMPUTE (NO IO / THREADS)
    │   ├── context/
    │   │   ├── ctx.c
    │   │
    │   ├── buffer/
    │   │   ├── buffer.h
    │   │   ├── buffer_lifecycle.c
    │   │   └── buffer_lifecycle.h
    │   │
    │   ├── pipeline/
    │   │   ├── pipeline_desc.c
    │   │   ├── pipeline_compile.c
    │   │
    │   ├── engine/
    │   │   ├── engine.c
    │   │
    │   └── config/
    │       └── config.c
    │
    ├── pipeline/                             # 🔥 HOT EXECUTION ONLY
    │   ├── exec/
    │   │   ├── pipeline_exec.c
    │   │
    │   ├── batch/
    │   │   ├── batch_exec.c
    │   │   ├── batch_builder.c
    │   │
    │   ├── threaded/
    │   │   ├── pipeline_threaded.c
    │   │
    │   └── dispatch/
    │       ├── jump_table.c
    │
    ├── runtime/                              # 🔥 EXECUTION + SCHEDULING
    │   ├── worker/
    │   │   ├── worker.c
    │   │
    │   ├── scheduler/
    │   │   ├── scheduler.c
    │   │
    │   ├── queue/
    │   │   ├── mpmc.c
    │   │   ├── spsc.c
    │   │
    │   ├── dispatch/
    │   │   ├── exec_router.c
    │   │
    │   ├── affinity/
    │   │   ├── affinity.c
    │   │
    │   └── cluster/                          # 🔥 CLUSTER RUNTIME
    │       ├── cluster_registry.c
    │
    ├── memory/                               # 🔥 MEMORY SUBSYSTEM
    │   ├── slab/
    │   │   ├── slab.c
    │   │
    │   ├── arena/
    │   │   ├── arena.c
    │   │
    │   ├── numa/
    │   │   ├── numa.c
    │   │
    │   └── hugepage/
    │       ├── hugepage.c
    │
    ├── io/                                   # 🔥 ALL IO (STRICTLY ISOLATED)
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
    ├── plugins/                              # 🔥 EXTENSIBILITY LAYER
    │   ├── builtin/
    │   │   ├── plugin_resize.c
    │   │   ├── plugin_crop.c
    │   │   ├── plugin_grayscale.c
    │   │   └── plugin_registry.c
    │   │
    │   └── dynamic/
    │       └── plugin_loader.c
    │
    ├── observability/                        # 🔥 PERFORMANCE + DEBUG POWER
    │   ├── binlog/
    │   │   ├── binlog.c
    │   │
    │   ├── tracing/
    │   │   ├── tracing.c
    │   │
    │   ├── metrics/
    │   │   ├── metrics.c
    │   │
    │   ├── logger/
    │   │   ├── logger.c
    │   │
    │   ├── profiler/
    │   │   ├── profiler.c
    │   │
    │   └── events/
    │       └── events.h
    │
    ├── security/                             # 🔥 HARDENING LAYER
    │   ├── sandbox/
    │   │   ├── sandbox.c
    │   │
    │   ├── validation/
    │   │   ├── input_validator.c
    │   │
    │   ├── bounds/
    │   │   └── bounds_check.c
    │   │
    │   ├── poison/
    │   │   └── poison.c
    │   │
    │   └── fuzz/
    │       └── fuzz_hooks.c
    │
    ├── arch/                                 # 🔥 HARDWARE BACKENDS
    │   ├── x86/
    │   │   ├── avx2/
    │   │   └── avx512/
    │   │
    │   ├── arm/
    │   │   └── neon/
    │   │
    │   └── cpu_caps.c
    │
    ├── cold/                                 # 🔥 NON-HOT UTILITIES
    │   ├── debug/
    │   │   ├── debug.c
    │   │
    │   ├── error/
    │   │   ├── error.c
    │   │
    │   └── validation/
    │       ├── validation.c
    │
    ├── cmd/                                  # 🔥 CLI + BENCHMARKING
    │   ├── imgengine/
    │   │   ├── main.c
    │   │   ├── args.c
    │   │   └── io_uring_engine.c
    │   │
    │   └── bench/
    │       └── lat_bench.c
    │

├── tests/                                # 🔥 REQUIRED (L10 QUALITY)
│   ├── unit/
│   ├── fuzz/
│   └── perf/
│
├── build/
├── CMakeLists.txt
└── README.md                             # 🔥 MUST INCLUDE MASTER BUILD ORDER
