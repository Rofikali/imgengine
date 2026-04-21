// ./include/hot/pipeline_exec.h
// include/hot/pipeline_exec.h

#ifndef IMGENGINE_HOT_PIPELINE_EXEC_H
#define IMGENGINE_HOT_PIPELINE_EXEC_H

#include "pipeline/pipeline_compiled.h"
#include "pipeline/fused_kernel.h"
#include "pipeline/fused_params.h"

/*
 * Forward declarations
 */
typedef struct img_pipeline_runtime img_pipeline_runtime_t;
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

/*
 * img_pipeline_execute_hot()
 *
 * Sequential single-image hot path.
 * Uses jump table dispatch per op.
 */
void img_pipeline_execute_hot(
    img_ctx_t *__restrict ctx,
    const img_pipeline_runtime_t *__restrict pipe,
    img_buffer_t *__restrict buf);

void img_pipeline_execute_compiled_hot(
    img_ctx_t *__restrict ctx,
    const img_pipeline_compiled_t *__restrict pipe,
    img_buffer_t *__restrict buf);

/*
 * img_pipeline_execute_fused()
 *
 * Fused single-image hot path.
 * fn: img_single_kernel_fn — takes (ctx, buf), NOT batch ABI.
 * params: pre-compiled fused param block (cold path).
 *
 * NEVER pass img_fused_kernel_fn here — batch ABI is incompatible.
 */
void img_pipeline_execute_fused(
    img_ctx_t *ctx,
    img_single_kernel_fn fn,
    img_fused_params_t *params,
    img_buffer_t *buf);

#endif /* IMGENGINE_HOT_PIPELINE_EXEC_H */
