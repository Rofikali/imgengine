// ./include/pipeline/fused_kernel.h
// include/pipeline/fused_kernel.h

#ifndef IMGENGINE_FUSED_KERNEL_H
#define IMGENGINE_FUSED_KERNEL_H

#include "core/context_internal.h"
#include "pipeline/batch.h"

/*
 * BATCH KERNEL ABI
 *
 * One call → entire batch.
 * params: pointer to img_fused_params_t
 *
 * Used by: fused_dispatch.c, batch_exec.c, pipeline_fuse.c
 */
typedef void (*img_fused_kernel_fn)(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *params);

/*
 * SINGLE-IMAGE KERNEL ABI
 *
 * One call → one img_buffer_t.
 * ctx->fused_params must be set before call.
 *
 * Used by: pipeline_exec.c (hot path single-image)
 *          pipeline_fuse.c (AVX2/scalar fused exec)
 */
typedef void (*img_single_kernel_fn)(
    img_ctx_t *ctx,
    img_buffer_t *buf);

/*
 * FUSED KERNEL DECLARATIONS
 *
 * Defined in: src/pipeline/fused_kernels.c
 * via DEFINE_KERNEL macro in fused_kernels.inc
 *
 * ALL kernels use BATCH ABI (img_fused_kernel_fn).
 * Single-image path uses AVX2/scalar in pipeline_fuse.c directly.
 *
 * ABI-STABLE: do not rename without MAJOR version bump.
 */
void kernel_none(img_ctx_t *, img_batch_t *, void *);
void kernel_gray(img_ctx_t *, img_batch_t *, void *);
void kernel_bright(img_ctx_t *, img_batch_t *, void *);
void kernel_gray_bright(img_ctx_t *, img_batch_t *, void *);
void kernel_gray_resize(img_ctx_t *, img_batch_t *, void *);
void kernel_all(img_ctx_t *, img_batch_t *, void *);

#endif /* IMGENGINE_FUSED_KERNEL_H */

// // include/pipeline/fused_kernel.h

// #ifndef IMGENGINE_FUSED_KERNEL_H
// #define IMGENGINE_FUSED_KERNEL_H

// #include "core/context_internal.h"
// #include "core/buffer.h"

// /*
//  * 🔥 FUSED PARAM BLOCK (CACHE-FRIENDLY)
//  */
// typedef struct
// {
//     void *blocks[8]; // per-op param blocks
//     uint32_t count;
// } img_fused_params_t;

// /*
//  * 🔥 FUSED KERNEL ABI (NO PARAM ARG)
//  */
// typedef void (*img_fused_kernel_fn)(
//     img_ctx_t *,
//     img_buffer_t *);

// #endif