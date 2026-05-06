// ./include/pipeline/layout_resize_internal.h

// ================================================================
// FILE 1: include/pipeline/layout_resize_internal.h  (UPDATE)
// Add img_ctx_t* to every resize function signature.
// ================================================================

#ifndef IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H
#define IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H

#include "core/result.h"
#include "core/buffer.h"
#include "core/context_internal.h" /* img_ctx_t — needed for ctx param */
#include "memory/slab.h"
#include <stdint.h>

/*
 * img_layout_resize_bilinear()
 *
 * Allocates output buffer from slab, then resamples src → dst
 * at (new_w × new_h) pixels.
 *
 * ctx: carries caps and local_pool for jump table dispatch.
 *      Must not be NULL.
 */
img_result_t img_layout_resize_bilinear(img_ctx_t *ctx, const img_buffer_t *src, img_buffer_t *dst,
                                        uint32_t new_w, uint32_t new_h, img_slab_pool_t *pool);

/*
 * img_layout_resize_resample_bilinear()
 *
 * Core pixel resampling. dst buffer already allocated by caller.
 * Dispatches to AVX2 kernel via jump table when available.
 * Falls back to scalar bilinear when jump table has no entry.
 *
 * ctx: used for jump table dispatch. Must not be NULL.
 */
void img_layout_resize_resample_bilinear(img_ctx_t *ctx, const img_buffer_t *src, uint8_t *mem,
                                         uint32_t new_w, uint32_t new_h, uint32_t stride);

/*
 * img_layout_resize_validate_alloc()
 *
 * Validates dimensions and allocates output buffer from slab.
 * Pure validation — no ctx needed.
 */
img_result_t img_layout_resize_validate_alloc(img_ctx_t *ctx, const img_buffer_t *src,
                                              img_buffer_t *dst, uint32_t new_w, uint32_t new_h,
                                              img_slab_pool_t *pool, uint8_t **mem_out,
                                              uint32_t *stride_out);

/*
 * img_layout_prepare_fill()
 * img_layout_prepare_fill_commit()
 * img_layout_prepare_fit()
 * img_layout_prepare_fit_commit()
 *
 * All accept ctx so the resize step can use the jump table.
 */
img_result_t img_layout_prepare_fill(img_ctx_t *ctx, const img_buffer_t *src, img_buffer_t *dst,
                                     uint32_t cell_w, uint32_t cell_h, img_slab_pool_t *pool);

img_result_t img_layout_prepare_fill_commit(const img_buffer_t *tmp, img_buffer_t *dst,
                                            img_slab_pool_t *pool, uint32_t cell_w,
                                            uint32_t cell_h);

img_result_t img_layout_prepare_fit(img_ctx_t *ctx, const img_buffer_t *src, img_buffer_t *dst,
                                    uint32_t cell_w, uint32_t cell_h, img_slab_pool_t *pool,
                                    uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);

img_result_t img_layout_prepare_fit_commit(const img_buffer_t *tmp, img_buffer_t *dst,
                                           img_slab_pool_t *pool, uint32_t cell_w, uint32_t cell_h,
                                           uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);

#endif /* IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H */

// // ./include/pipeline/layout_resize_internal.h
// #ifndef IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H
// #define IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H

// #include "pipeline/layout_resize.h"

// img_result_t img_layout_resize_validate_alloc(
//     const img_buffer_t *src,
//     img_buffer_t *dst,
//     uint32_t new_w,
//     uint32_t new_h,
//     img_slab_pool_t *pool,
//     uint8_t **mem_out,
//     uint32_t *stride_out);

// void img_layout_resize_resample_bilinear(
//     const img_buffer_t *src,
//     uint8_t *mem,
//     uint32_t new_w,
//     uint32_t new_h,
//     uint32_t stride);

// img_result_t img_layout_prepare_fill_commit(
//     const img_buffer_t *tmp,
//     img_buffer_t *dst,
//     img_slab_pool_t *pool,
//     uint32_t cell_w,
//     uint32_t cell_h);

// img_result_t img_layout_prepare_fit_commit(
//     const img_buffer_t *tmp,
//     img_buffer_t *dst,
//     img_slab_pool_t *pool,
//     uint32_t cell_w,
//     uint32_t cell_h,
//     uint8_t bg_r,
//     uint8_t bg_g,
//     uint8_t bg_b);

// #endif /* IMGENGINE_PIPELINE_LAYOUT_RESIZE_INTERNAL_H */
