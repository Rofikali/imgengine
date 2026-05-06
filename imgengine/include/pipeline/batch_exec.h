// ./include/pipeline/batch_exec.h
// include/pipeline/batch_exec.h

// ================================================================
// FILE: include/pipeline/batch_exec.h
// CHANGE: rename img_batch_execute → img_batch_execute_fused
//         "fused" = fused kernel path (profiling + tracepoints)
// ================================================================

#ifndef IMGENGINE_BATCH_EXEC_H
#define IMGENGINE_BATCH_EXEC_H

#include "core/context_internal.h"
#include "pipeline/batch.h"

typedef void (*img_fused_kernel_fn)(img_ctx_t *ctx, img_batch_t *batch, void *params);

/*
 * img_batch_execute_fused()
 *
 * Hot path: batch execution via fused pipeline.
 * pipeline: pointer to img_pipeline_fused_t (pre-compiled).
 * Includes RDTSC profiling and tracepoint hooks.
 * Used when pipeline ops have been fused into a single kernel.
 */
void img_batch_execute_fused(img_ctx_t *ctx, img_batch_t *batch, void *pipeline);

#endif /* IMGENGINE_BATCH_EXEC_H */

// // ./include/pipeline/batch_exec.h

// #ifndef IMGENGINE_BATCH_EXEC_H
// #define IMGENGINE_BATCH_EXEC_H

// #include "core/context_internal.h"
// #include "pipeline/batch.h"
// // #include "api/v1/img_types.h"

// typedef void (*img_fused_kernel_fn)(
//     img_ctx_t *ctx,
//     img_batch_t *batch,
//     void *params);

// void img_batch_execute(
//     img_ctx_t *ctx,
//     img_batch_t *batch,
//     void *pipeline);

// #endif