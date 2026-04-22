
// ================================================================
// FILE 9: src/pipeline/layout_grid.c  (UPDATE)
// Thread ctx through — the topmost function of the chain.
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout_grid_internal.h"

img_result_t img_layout_grid(
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    const img_buffer_t *photo,
    const img_job_t *job,
    img_layout_t *layout,
    img_arena_t *arena,
    img_slab_pool_t *pool)
{
    if (!ctx || !canvas || !photo || !job || !layout || !arena || !pool)
        return IMG_ERR_SECURITY;

    layout->cells = NULL;
    layout->count = 0;

    if (job->cols == 0 || job->rows == 0)
        return IMG_ERR_SECURITY;

    uint32_t pw = canvas->photo_w_px;
    uint32_t ph = canvas->photo_h_px;

    img_buffer_t scaled = {0};
    uint8_t scaled_owned = 0;
    img_result_t r = img_layout_grid_prepare_scaled(
        ctx, photo, job, pw, ph, pool, &scaled, &scaled_owned); /* ctx threaded */
    if (r != IMG_SUCCESS)
        return r;

    img_cell_t *cells = NULL;
    r = img_layout_grid_alloc_cells(job, arena, &cells);
    if (r != IMG_SUCCESS)
    {
        if (scaled_owned)
            img_slab_recycle(pool, scaled.data);
        return r;
    }

    if (job->gap > UINT32_MAX - pw || job->gap > UINT32_MAX - ph)
    {
        if (scaled_owned)
            img_slab_recycle(pool, scaled.data);
        return IMG_ERR_SECURITY;
    }

    layout->cells = cells;
    layout->count = img_layout_grid_place_cells(canvas, &scaled, job, cells);
    if (scaled_owned)
        img_slab_recycle(pool, scaled.data);
    return IMG_SUCCESS;
}

// // ./src/pipeline/layout_grid.c
// #define _GNU_SOURCE

// #include "pipeline/layout_grid_internal.h"

// img_result_t img_layout_grid(
//     img_canvas_t *canvas,
//     const img_buffer_t *photo,
//     const img_job_t *job,
//     img_layout_t *layout,
//     img_arena_t *arena,
//     img_slab_pool_t *pool)
// {
//     if (!canvas || !photo || !job || !layout || !arena || !pool)
//         return IMG_ERR_SECURITY;

//     layout->cells = NULL;
//     layout->count = 0;

//     if (job->cols == 0 || job->rows == 0)
//         return IMG_ERR_SECURITY;

//     uint32_t pw = canvas->photo_w_px;
//     uint32_t ph = canvas->photo_h_px;

//     img_buffer_t scaled = {0};
//     img_result_t r = img_layout_grid_prepare_scaled(
//         photo, job, pw, ph, pool, &scaled);
//     if (r != IMG_SUCCESS)
//         return r;

//     img_cell_t *cells = NULL;
//     r = img_layout_grid_alloc_cells(job, arena, &cells);
//     if (r != IMG_SUCCESS)
//     {
//         img_slab_recycle(pool, scaled.data);
//         return r;
//     }

//     if (job->gap > UINT32_MAX - pw || job->gap > UINT32_MAX - ph)
//     {
//         img_slab_recycle(pool, scaled.data);
//         return IMG_ERR_SECURITY;
//     }

//     layout->cells = cells;
//     layout->count = img_layout_grid_place_cells(canvas, &scaled, job, cells);
//     img_slab_recycle(pool, scaled.data);
//     return IMG_SUCCESS;
// }
