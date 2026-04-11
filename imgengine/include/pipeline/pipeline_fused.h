// ./include/pipeline/pipeline_fused.h
// include/pipeline/pipeline_fused.h

#ifndef IMGENGINE_PIPELINE_FUSED_H
#define IMGENGINE_PIPELINE_FUSED_H

#include <stdint.h>
#include "pipeline/fused_kernel.h"
#include "pipeline/fused_params.h"

/*
 * Forward declarations
 */
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_batch img_batch_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

/*
 * FUSED OP IR TAGS
 * Used by scalar reference path only.
 * Hot path uses pre-compiled fn pointers.
 */
typedef enum
{
    FUSED_OP_NONE = 0,
    FUSED_OP_GRAYSCALE = 1,
    FUSED_OP_BRIGHTNESS = 2,
    FUSED_OP_RESIZE = 3,
} img_fused_op_t;

#define IMG_MAX_FUSED_OPS 16

/*
 * COMPILED FUSED PIPELINE
 *
 * Produced once by img_pipeline_fuse() (cold path).
 * Used repeatedly by execute functions (hot path).
 *
 * fn:       single-image ABI — (ctx, buf)
 *           set to fused_exec_avx2 or fused_exec_scalar
 *
 * fn_batch: batch ABI — (ctx, batch, params)
 *           set from dispatch table, or NULL if no fused batch kernel
 *           for this op combination
 *
 * 64-byte aligned: hot fields fit in first cache line.
 */
typedef struct __attribute__((aligned(64)))
{
    uint32_t count;

    img_single_kernel_fn fn;      /* single-image exec (always set)  */
    img_fused_kernel_fn fn_batch; /* batch exec (NULL = no fused)    */

} img_pipeline_fused_t;

/*
 * Pipeline signature type (bitmask of active ops)
 */
typedef uint32_t img_pipeline_sig_t;

/*
 * API
 */
int img_pipeline_fuse(
    const img_pipeline_desc_t *in,
    img_pipeline_fused_t *out);

void img_pipeline_execute_fused(
    img_ctx_t *ctx,
    img_pipeline_fused_t *pipe,
    img_buffer_t *buf);

// void img_pipeline_execute_fused(
//     img_ctx_t *ctx,
//     img_single_kernel_fn fn,
//     img_fused_params_t *params,
//     img_buffer_t *buf);

void img_pipeline_execute_fused_batch(
    img_ctx_t *ctx,
    img_pipeline_fused_t *pipe,
    img_batch_t *batch);

/*
 * Dispatch resolver
 * Defined in: src/pipeline/fused_dispatch.c
 */
img_fused_kernel_fn img_get_fused_kernel(img_pipeline_sig_t sig);

#endif /* IMGENGINE_PIPELINE_FUSED_H */
