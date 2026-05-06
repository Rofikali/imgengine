// ./src/pipeline/batch_exec.c

// ================================================================
// FILE: src/pipeline/batch_exec.c  (UPDATE)
// Replace direct observability includes with pipeline/trace.h
// ================================================================

// src/pipeline/batch_exec.c

#define _GNU_SOURCE

#include "pipeline/batch_exec.h"
#include "pipeline/pipeline_fused.h"
#include "pipeline/pipeline_signature.h"
#include "pipeline/fused_params.h"
#include "pipeline/trace.h" /* replaces all observability/ includes */
#include "core/buffer.h"
#include <stddef.h>

void img_batch_execute_fused(img_ctx_t *ctx, img_batch_t *batch, void *pipeline) {
    if (__builtin_expect(!batch || batch->count == 0, 0))
        return;

    img_pipeline_fused_t *pipe = (img_pipeline_fused_t *)pipeline;
    if (__builtin_expect(!pipe, 0))
        return;

    const uint64_t start = PIPELINE_PROFILER_NOW();

    if (__builtin_expect(pipe->fn_batch != NULL, 1)) {
        pipe->fn_batch(ctx, batch, ctx->fused_params);
    } else {
        for (uint32_t i = 0; i < batch->count; i++) {
            if (__builtin_expect(batch->buffers[i] != NULL, 1))
                pipe->fn(ctx, batch->buffers[i]);
        }
    }

    const uint64_t end = PIPELINE_PROFILER_NOW();
    const uint64_t cycles = end - start;

    PIPELINE_LOG_LATENCY(cycles, batch->count, 0);
    PIPELINE_TRACE("batch_exec_fused", cycles, batch->count, 0);
}

// // ./src/pipeline/batch_exec.c

// // ================================================================
// // FILE: src/pipeline/batch_exec.c
// // CHANGE: rename function definition to img_batch_execute_fused
// // ================================================================

// #include "pipeline/batch_exec.h"
// #include "pipeline/pipeline_fused.h"
// #include "pipeline/pipeline_signature.h"
// #include "pipeline/fused_params.h"
// #include "observability/profiler.h"
// #include "observability/logger.h"
// #include "observability/binlog_fast.h"
// #include "observability/tracepoints.h"
// #include "core/buffer.h"
// #include <stddef.h>

// /*
//  * img_batch_execute_fused()
//  *
//  * Hot path: batch execution via fused pipeline.
//  * Resolves pre-compiled fused kernel and dispatches entire batch
//  * in a single call. Falls back to sequential single-image if no
//  * fused kernel available for this op combination.
//  *
//  * CONTRACT:
//  *   - pipeline is img_pipeline_fused_t* (pre-compiled, cold path)
//  *   - batch->buffers[i] non-NULL for i < batch->count
//  *   - No allocation, no locks
//  */
// void img_batch_execute_fused(
//     img_ctx_t *ctx,
//     img_batch_t *batch,
//     void *pipeline)
// {
//     if (__builtin_expect(!batch || batch->count == 0, 0))
//         return;

//     img_pipeline_fused_t *pipe = (img_pipeline_fused_t *)pipeline;

//     if (__builtin_expect(!pipe, 0))
//         return;

//     const uint64_t start = img_profiler_now();

//     if (__builtin_expect(pipe->fn_batch != NULL, 1))
//     {
//         pipe->fn_batch(ctx, batch, ctx->fused_params);
//     }
//     else
//     {
//         for (uint32_t i = 0; i < batch->count; i++)
//         {
//             if (__builtin_expect(batch->buffers[i] != NULL, 1))
//                 pipe->fn(ctx, batch->buffers[i]);
//         }
//     }

//     const uint64_t end = img_profiler_now();
//     const uint64_t cycles = end - start;

//     IMG_LOG_LATENCY(cycles, batch->count, 0);
//     IMG_TRACE("batch_exec_fused", cycles, batch->count, 0);
// }
