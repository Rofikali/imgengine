
// ================================================================
// FILE 6: src/pipeline/layout_prepare_fill.c  (UPDATE)
// Thread ctx through.
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"

img_result_t img_layout_prepare_fill(
    img_ctx_t *ctx,
    const img_buffer_t *src,
    img_buffer_t *dst,
    uint32_t cell_w,
    uint32_t cell_h,
    img_slab_pool_t *pool)
{
    if (!ctx || !src || !dst || !pool || !src->data || src->width == 0 || src->height == 0)
        return IMG_ERR_SECURITY;

    float sx = (float)cell_w / (float)src->width;
    float sy = (float)cell_h / (float)src->height;
    float s = (sx > sy) ? sx : sy;
    uint32_t scaled_w = (uint32_t)((float)src->width * s);
    uint32_t scaled_h = (uint32_t)((float)src->height * s);

    if (scaled_w < cell_w)
        scaled_w = cell_w;
    if (scaled_h < cell_h)
        scaled_h = cell_h;

    img_buffer_t tmp = {0};
    img_result_t r = img_layout_resize_bilinear(
        ctx, src, &tmp, scaled_w, scaled_h, pool);
    if (r != IMG_SUCCESS)
        return r;

    r = img_layout_prepare_fill_commit(&tmp, dst, pool, cell_w, cell_h);
    img_slab_recycle(pool, tmp.data);
    return r;
}

// // ./src/pipeline/layout_prepare_fill.c
// #define _GNU_SOURCE

// #include "pipeline/layout_resize_internal.h"

// img_result_t img_layout_prepare_fill(
//     const img_buffer_t *src,
//     img_buffer_t *dst,
//     uint32_t cell_w,
//     uint32_t cell_h,
//     img_slab_pool_t *pool)
// {
//     if (!src || !dst || !pool || !src->data || src->width == 0 || src->height == 0)
//         return IMG_ERR_SECURITY;

//     float sx = (float)cell_w / (float)src->width;
//     float sy = (float)cell_h / (float)src->height;
//     float s = (sx > sy) ? sx : sy;

//     uint32_t scaled_w = (uint32_t)((float)src->width * s);
//     uint32_t scaled_h = (uint32_t)((float)src->height * s);
//     if (scaled_w < cell_w)
//         scaled_w = cell_w;
//     if (scaled_h < cell_h)
//         scaled_h = cell_h;

//     img_buffer_t tmp = {0};
//     img_result_t r = img_layout_resize_bilinear(src, &tmp, scaled_w, scaled_h, pool);
//     if (r != IMG_SUCCESS)
//         return r;

//     r = img_layout_prepare_fill_commit(&tmp, dst, pool, cell_w, cell_h);
//     img_slab_recycle(pool, tmp.data);
//     return r;
// }
