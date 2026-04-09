#!/usr/bin/env bash
set -e

echo "🚀 Creating imgengine structure..."

# Root folder
mkdir -p imgengine
cd imgengine

# ---------------------------
# DIRECTORIES
# ---------------------------
mkdir -p api/v1

mkdir -p core/context core/buffer core/pipeline core/engine core/config

mkdir -p pipeline/exec pipeline/batch pipeline/threaded pipeline/dispatch

mkdir -p runtime/worker runtime/scheduler runtime/queue runtime/dispatch runtime/affinity runtime/cluster

mkdir -p memory/slab memory/arena memory/numa memory/hugepage

mkdir -p io/decoder io/encoder io/vfs io/remote

mkdir -p plugins/builtin plugins/dynamic

mkdir -p observability/binlog observability/tracing observability/metrics observability/logger observability/profiler observability/events

mkdir -p security/sandbox security/validation security/bounds security/poison security/fuzz

mkdir -p arch/x86/avx2 arch/x86/avx512 arch/arm/neon

mkdir -p cold/debug cold/error cold/validation

mkdir -p cmd/imgengine cmd/bench

mkdir -p tests/unit tests/fuzz tests/perf

mkdir -p build

# ---------------------------
# FILES
# ---------------------------

# API
touch api/v1/img_api.h
touch api/v1/img_pipeline.h
touch api/v1/img_buffer_utils.h
touch api/v1/img_plugin_api.h
touch api/v1/img_error.h

# CORE
touch core/context/ctx.c core/context/ctx.h core/context/ctx_internal.h
touch core/buffer/buffer.h core/buffer/buffer_lifecycle.c core/buffer/buffer_lifecycle.h
touch core/pipeline/pipeline_desc.c core/pipeline/pipeline_compile.c core/pipeline/pipeline_types.h core/pipeline/opcodes.h
touch core/engine/engine.c core/engine/engine.h
touch core/config/config.c core/config/config.h

# PIPELINE
touch pipeline/exec/pipeline_exec.c pipeline/exec/pipeline_exec.h
touch pipeline/batch/batch_exec.c pipeline/batch/batch_builder.c pipeline/batch/batch.h
touch pipeline/threaded/pipeline_threaded.c pipeline/threaded/pipeline_threaded.h
touch pipeline/dispatch/jump_table.c pipeline/dispatch/jump_table.h

# RUNTIME
touch runtime/worker/worker.c runtime/worker/worker.h
touch runtime/scheduler/scheduler.c runtime/scheduler/scheduler.h
touch runtime/queue/mpmc.c runtime/queue/mpmc.h runtime/queue/spsc.c runtime/queue/spsc.h
touch runtime/dispatch/exec_router.c runtime/dispatch/exec_router.h
touch runtime/affinity/affinity.c runtime/affinity/affinity.h
touch runtime/cluster/cluster_registry.c runtime/cluster/cluster_registry.h

# MEMORY
touch memory/slab/slab.c memory/slab/slab.h memory/slab/slab_internal.h
touch memory/arena/arena.c memory/arena/arena.h
touch memory/numa/numa.c memory/numa/numa.h
touch memory/hugepage/hugepage.c memory/hugepage/hugepage.h

# IO
touch io/decoder/decoder_entry.c io/decoder/streaming_decoder.c
touch io/encoder/encoder_entry.c
touch io/vfs/memory_stream.c io/vfs/http_stream.c
touch io/remote/remote_fetch.c

# PLUGINS
touch plugins/builtin/plugin_resize.c plugins/builtin/plugin_crop.c plugins/builtin/plugin_grayscale.c plugins/builtin/plugin_registry.c
touch plugins/dynamic/plugin_loader.c

# OBSERVABILITY
touch observability/binlog/binlog.c observability/binlog/binlog.h observability/binlog/binlog_fast.h
touch observability/tracing/tracing.c observability/tracing/tracing.h observability/tracing/tracepoints.h
touch observability/metrics/metrics.c observability/metrics/metrics.h
touch observability/logger/logger.c observability/logger/logger.h
touch observability/profiler/profiler.c observability/profiler/profiler.h
touch observability/events/events.h

# SECURITY
touch security/sandbox/sandbox.c security/sandbox/sandbox.h
touch security/validation/input_validator.c security/validation/input_validator.h
touch security/bounds/bounds_check.h
touch security/poison/poison.h
touch security/fuzz/fuzz_hooks.c

# ARCH
touch arch/cpu_caps.c

# COLD
touch cold/debug/debug.c cold/debug/debug.h
touch cold/error/error.c cold/error/error.h
touch cold/validation/validation.c cold/validation/validation.h

# CMD
touch cmd/imgengine/main.c cmd/imgengine/args.c cmd/imgengine/io_uring_engine.c
touch cmd/bench/lat_bench.c

# ROOT FILES
touch CMakeLists.txt README.md

echo "✅ Done!"
