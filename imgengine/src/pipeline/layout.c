// src/pipeline/layout.c
//
// FIX: Gap was computed in cell step but photos were resized to
// canvas->photo_w_px which filled the entire cell.
// The resize must use the PHOTO size, not the CELL size.
// Cell size = photo size. Step between cells = photo + gap.
// Gap = white background showing between blitted photos.
//
// canvas->photo_w_px is the photo pixel size (correct).
// Blit step is photo_w + gap (correct).
// So gap IS the white background between photos — but only if
// the canvas background is white and photos don't overflow.
// The issue was canvas->photo_w_px was being computed as
// avail_w / cols which INCLUDES the gap space, making photos
// touch each other.
//
// CORRECT formula:
//   avail_w = page_w - 2*padding - (cols-1)*gap
//   photo_w  = avail_w / cols
//   step     = photo_w + gap
//   grid_w   = cols * photo_w + (cols-1) * gap

#define _GNU_SOURCE

#include "pipeline/layout.h"
#include "pipeline/canvas.h"
#include "core/buffer.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include "api/v1/img_error.h"
#include <string.h>

extern void img_blit_avx2(img_buffer_t *, const img_buffer_t *,
                          uint32_t, uint32_t);

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
    size_t block = img_slab_block_size(pool);

    if (required > block)
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

            const uint8_t *sp = src->data + (size_t)sy * src->stride + (size_t)sx * 3;
            uint8_t *dp = mem + (size_t)dy * dst->stride + (size_t)dx * 3;

            dp[0] = sp[0];
            dp[1] = sp[1];
            dp[2] = sp[2];
        }
    }

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

    img_buffer_t scaled = {0};
    img_result_t r = resize_nearest(photo, &scaled, pw, ph, pool);
    if (r != IMG_SUCCESS)
        return r;

    uint32_t total = job->cols * job->rows;
    img_cell_t *cells = img_arena_alloc(arena, sizeof(img_cell_t) * total);
    if (!cells)
    {
        img_slab_free(pool, scaled.data);
        return IMG_ERR_NOMEM;
    }

    /*
     * STEP = photo + gap
     * Gap is WHITE BACKGROUND between photos.
     * Photos are blitted at exact cell origin.
     * Background fill in canvas_init() provides the white gap.
     */
    // uint32_t step_x = pw + job->gap;
    // uint32_t step_y = ph + job->gap;

    // for (uint32_t row = 0; row < job->rows; row++)
    // {
    //     for (uint32_t col = 0; col < job->cols; col++)
    //     {
    //         uint32_t x = canvas->start_x + col * step_x;
    //         uint32_t y = canvas->start_y + row * step_y;

    //         /* strict bounds: never blit past canvas edge */
    //         if (x + pw > canvas->buf.width)
    //             continue;
    //         if (y + ph > canvas->buf.height)
    //             continue;

    //         img_blit_avx2(&canvas->buf, &scaled, x, y);

    //         uint32_t idx = row * job->cols + col;
    //         cells[idx].x = x;
    //         cells[idx].y = y;
    //         cells[idx].w = pw;
    //         cells[idx].h = ph;
    //     }
    // }
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
