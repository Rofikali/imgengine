// src/pipeline/layout.c

// ================================================================
// FILE 1: src/pipeline/layout.c
// Wire job->mode: IMG_FILL = center-crop to fill cell exactly
//                 IMG_FIT  = scale to fit, letterbox with bg color
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout.h"
#include "pipeline/canvas.h"
#include "core/buffer.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include "pipeline/job.h"
#include "core/result.h"
#include <string.h>

extern void img_blit_avx2(img_buffer_t *, const img_buffer_t *,
                          uint32_t, uint32_t);

/*
 * resize_nearest()
 * Nearest-neighbour resize. src→dst at new_w×new_h.
 */
static img_result_t resize_nearest(
    const img_buffer_t *src,
    img_buffer_t *dst,
    uint32_t new_w,
    uint32_t new_h,
    img_slab_pool_t *pool)
{
    if (new_w == 0 || new_h == 0)
        return IMG_ERR_FORMAT;

    size_t required = (size_t)new_w * new_h * 3;
    if (required > img_slab_block_size(pool))
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    dst->data = mem;
    dst->width = new_w;
    dst->height = new_h;
    dst->channels = 3;
    dst->stride = new_w * 3;

    for (uint32_t dy = 0; dy < new_h; dy++)
    {
        for (uint32_t dx = 0; dx < new_w; dx++)
        {
            uint32_t sx = (dx * src->width) / new_w;
            uint32_t sy = (dy * src->height) / new_h;
            if (sx >= src->width)
                sx = src->width - 1;
            if (sy >= src->height)
                sy = src->height - 1;
            const uint8_t *sp = src->data + sy * src->stride + sx * 3;
            uint8_t *dp = mem + dy * dst->stride + dx * 3;
            dp[0] = sp[0];
            dp[1] = sp[1];
            dp[2] = sp[2];
        }
    }
    return IMG_SUCCESS;
}

/*
 * prepare_fill()
 *
 * IMG_FILL: scale photo so it COVERS the cell completely,
 * then center-crop to exact cell size.
 *
 * Scale factor = max(cell_w/src_w, cell_h/src_h)
 * After scale: one dimension matches cell exactly, other is >=cell.
 * Crop: take center slice of the oversized dimension.
 *
 * Result: no letterbox, no distortion, photo fills cell edge-to-edge.
 * Standard passport/ID photo behavior.
 */
static img_result_t prepare_fill(
    const img_buffer_t *src,
    img_buffer_t *dst,
    uint32_t cell_w,
    uint32_t cell_h,
    img_slab_pool_t *pool)
{
    float sx = (float)cell_w / (float)src->width;
    float sy = (float)cell_h / (float)src->height;
    float s = (sx > sy) ? sx : sy; /* scale to COVER */

    uint32_t scaled_w = (uint32_t)((float)src->width * s);
    uint32_t scaled_h = (uint32_t)((float)src->height * s);

    if (scaled_w < cell_w)
        scaled_w = cell_w;
    if (scaled_h < cell_h)
        scaled_h = cell_h;

    /* resize to scaled size */
    img_buffer_t tmp = {0};
    img_result_t r = resize_nearest(src, &tmp, scaled_w, scaled_h, pool);
    if (r != IMG_SUCCESS)
        return r;

    /* center-crop to cell size */
    uint32_t cx = (scaled_w - cell_w) / 2;
    uint32_t cy = (scaled_h - cell_h) / 2;

    size_t required = (size_t)cell_w * cell_h * 3;
    if (required > img_slab_block_size(pool))
    {
        img_slab_free(pool, tmp.data);
        return IMG_ERR_NOMEM;
    }

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
    {
        img_slab_free(pool, tmp.data);
        return IMG_ERR_NOMEM;
    }

    dst->data = mem;
    dst->width = cell_w;
    dst->height = cell_h;
    dst->channels = 3;
    dst->stride = cell_w * 3;

    for (uint32_t dy = 0; dy < cell_h; dy++)
    {
        const uint8_t *src_row = tmp.data + (cy + dy) * tmp.stride + cx * 3;
        uint8_t *dst_row = mem + dy * dst->stride;
        memcpy(dst_row, src_row, cell_w * 3);
    }

    img_slab_free(pool, tmp.data);
    return IMG_SUCCESS;
}

/*
 * prepare_fit()
 *
 * IMG_FIT: scale photo to fit WITHIN the cell, preserving aspect ratio.
 * Letterbox with background color on unused edges.
 *
 * Scale factor = min(cell_w/src_w, cell_h/src_h)
 * Photo is centered in the cell. Unused area = canvas background color.
 *
 * Standard "fit to frame" behavior.
 */
static img_result_t prepare_fit(
    const img_buffer_t *src,
    img_buffer_t *dst,
    uint32_t cell_w,
    uint32_t cell_h,
    img_slab_pool_t *pool,
    uint8_t bg_r, uint8_t bg_g, uint8_t bg_b)
{
    float sx = (float)cell_w / (float)src->width;
    float sy = (float)cell_h / (float)src->height;
    float s = (sx < sy) ? sx : sy; /* scale to FIT */

    uint32_t fit_w = (uint32_t)((float)src->width * s);
    uint32_t fit_h = (uint32_t)((float)src->height * s);

    if (fit_w == 0)
        fit_w = 1;
    if (fit_h == 0)
        fit_h = 1;
    if (fit_w > cell_w)
        fit_w = cell_w;
    if (fit_h > cell_h)
        fit_h = cell_h;

    /* allocate full cell, fill with background */
    size_t required = (size_t)cell_w * cell_h * 3;
    if (required > img_slab_block_size(pool))
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    for (size_t i = 0; i < required; i += 3)
    {
        mem[i] = bg_r;
        mem[i + 1] = bg_g;
        mem[i + 2] = bg_b;
    }

    dst->data = mem;
    dst->width = cell_w;
    dst->height = cell_h;
    dst->channels = 3;
    dst->stride = cell_w * 3;

    /* resize photo to fit size */
    img_buffer_t tmp = {0};
    img_result_t r = resize_nearest(src, &tmp, fit_w, fit_h, pool);
    if (r != IMG_SUCCESS)
    {
        img_slab_free(pool, mem);
        return r;
    }

    /* blit centered into cell */
    uint32_t off_x = (cell_w - fit_w) / 2;
    uint32_t off_y = (cell_h - fit_h) / 2;

    for (uint32_t dy = 0; dy < fit_h; dy++)
    {
        uint8_t *dst_row = mem + (off_y + dy) * dst->stride + off_x * 3;
        const uint8_t *src_row = tmp.data + dy * tmp.stride;
        memcpy(dst_row, src_row, fit_w * 3);
    }

    img_slab_free(pool, tmp.data);
    return IMG_SUCCESS;
}

img_result_t img_layout_grid(
    img_canvas_t *canvas,
    const img_buffer_t *photo,
    const img_job_t *job,
    img_layout_t *layout,
    img_arena_t *arena,
    img_slab_pool_t *pool)
{
    if (!canvas || !photo || !job || !layout || !arena || !pool)
        return IMG_ERR_SECURITY;

    uint32_t pw = canvas->photo_w_px;
    uint32_t ph = canvas->photo_h_px;

    /*
     * Prepare scaled photo according to job->mode.
     * IMG_FILL: center-crop to fill cell (default, passport standard)
     * IMG_FIT:  letterbox to fit within cell
     */
    img_buffer_t scaled = {0};
    img_result_t r;

    if (job->mode == IMG_FIT)
    {
        r = prepare_fit(photo, &scaled, pw, ph, pool,
                        job->bg_r, job->bg_g, job->bg_b);
    }
    else
    {
        /* IMG_FILL — default */
        r = prepare_fill(photo, &scaled, pw, ph, pool);
    }

    if (r != IMG_SUCCESS)
        return r;

    uint32_t total = job->cols * job->rows;
    img_cell_t *cells = img_arena_alloc(arena, sizeof(img_cell_t) * total);
    if (!cells)
    {
        img_slab_free(pool, scaled.data);
        return IMG_ERR_NOMEM;
    }

    uint32_t step_x = pw + job->gap;
    uint32_t step_y = ph + job->gap;

    for (uint32_t row = 0; row < job->rows; row++)
    {
        for (uint32_t col = 0; col < job->cols; col++)
        {
            uint32_t x = canvas->start_x + col * step_x;
            uint32_t y = canvas->start_y + row * step_y;

            if (x + pw > canvas->buf.width)
                continue;
            if (y + ph > canvas->buf.height)
                continue;

            img_blit_avx2(&canvas->buf, &scaled, x, y);

            uint32_t idx = row * job->cols + col;
            cells[idx].x = x;
            cells[idx].y = y;
            cells[idx].w = pw;
            cells[idx].h = ph;
        }
    }

    img_slab_free(pool, scaled.data);
    layout->cells = cells;
    layout->count = total;
    return IMG_SUCCESS;
}
