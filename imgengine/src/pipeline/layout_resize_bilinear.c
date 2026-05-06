// ./src/pipeline/layout_resize_bilinear.c

// ================================================================
// FILE 5: src/pipeline/layout_resize_bilinear.c  (UPDATE)
// Thread ctx through.
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"

img_result_t img_layout_resize_bilinear(img_ctx_t *ctx, const img_buffer_t *src, img_buffer_t *dst,
                                        uint32_t new_w, uint32_t new_h, img_slab_pool_t *pool) {
    uint8_t *mem = NULL;
    uint32_t stride = 0;

    img_result_t r =
        img_layout_resize_validate_alloc(ctx, src, dst, new_w, new_h, pool, &mem, &stride);
    if (r != IMG_SUCCESS)
        return r;

    dst->data = mem;
    dst->width = new_w;
    dst->height = new_h;
    dst->channels = 3;
    dst->stride = stride;

    /* ctx threaded through to the jump table dispatch */
    img_layout_resize_resample_bilinear(ctx, src, mem, new_w, new_h, stride);
    return IMG_SUCCESS;
}

// // ./src/pipeline/layout_resize_bilinear.c
// #define _GNU_SOURCE

// #include "pipeline/layout_resize_internal.h"

// img_result_t img_layout_resize_bilinear(
//     const img_buffer_t *src,
//     img_buffer_t *dst,
//     uint32_t new_w,
//     uint32_t new_h,
//     img_slab_pool_t *pool)
// {
//     uint8_t *mem = NULL;
//     uint32_t stride = 0;
//     img_result_t r = img_layout_resize_validate_alloc(
//         src, dst, new_w, new_h, pool, &mem, &stride);
//     if (r != IMG_SUCCESS)
//         return r;

//     dst->data = mem;
//     dst->width = new_w;
//     dst->height = new_h;
//     dst->channels = 3;
//     dst->stride = stride;

//     img_layout_resize_resample_bilinear(src, mem, new_w, new_h, stride);
//     return IMG_SUCCESS;
// }
