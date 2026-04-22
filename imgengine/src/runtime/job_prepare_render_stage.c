
// ================================================================
// FILE 1: src/runtime/job_prepare_render_stage.c  (REPLACE)
//
// WAS:
//   img_layout_grid(canvas, photo, job, layout, *arena, pool)
//
// FIX:
//   img_layout_grid(ctx, canvas, photo, job, layout, *arena, pool)
//
// ctx is built here from engine state and threaded down.
// This is the only caller of img_layout_grid in the hot path.
// ================================================================

#include "runtime/job_exec.h"
#include "core/context_internal.h"
#include "pipeline/canvas_internal.h"
#include "pipeline/render_cache.h"

#include <string.h>

img_result_t img_runtime_prepare_render_stage(
    img_engine_t *engine,
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    img_layout_t *layout,
    const img_job_t *job,
    const img_buffer_t *photo,
    img_arena_t **arena)
{
    if (!engine || !ctx || !canvas || !layout || !job || !photo || !arena)
        return IMG_ERR_SECURITY;

    img_render_cache_t *cache = (img_render_cache_t *)ctx->op_params;

    uint32_t pw = img_cm_to_px(IMG_A4_W_CM, job->dpi);
    uint32_t ph = img_cm_to_px(IMG_A4_H_CM, job->dpi);

    img_canvas_t probe = {0};
    probe.page_w_px = pw;
    probe.page_h_px = ph;
    img_canvas_compute_geometry(&probe, job, pw, ph);

    uint32_t render_sig =
        img_render_cache_signature(photo, job, probe.photo_w_px, probe.photo_h_px);

    if (cache && cache->final_valid &&
        cache->photo == photo &&
        cache->cell_w == probe.photo_w_px &&
        cache->cell_h == probe.photo_h_px &&
        cache->job_sig == render_sig &&
        cache->canvas.buf.data &&
        cache->layout.cells)
    {
        *canvas = cache->canvas;
        *layout = cache->layout;
        canvas->cache_owned = 1;
        return IMG_SUCCESS;
    }

    if (!*arena)
    {
        *arena = img_arena_create(1 * 1024 * 1024);
        if (!*arena)
            return IMG_ERR_NOMEM;
    }
    else
    {
        img_arena_reset(*arena);
    }

    img_result_t r = img_canvas_init(canvas, engine->global_pool, job);
    if (r != IMG_SUCCESS)
        return r;

    r = img_layout_grid(
        ctx,
        canvas,
        photo,
        job,
        layout,
        *arena,
        engine->global_pool);

    if (r == IMG_SUCCESS && cache)
    {
        size_t canvas_bytes = (size_t)canvas->buf.stride * canvas->buf.height;
        size_t layout_bytes = (size_t)layout->count * sizeof(img_cell_t);

        if (canvas_bytes > 0 &&
            canvas_bytes <= img_slab_block_size(engine->global_pool))
        {
            if (cache->final_valid)
                img_render_cache_discard(cache);

            if (!cache->layout_arena)
            {
                cache->layout_arena = img_arena_create(1024 * 1024);
            }
            else
            {
                img_arena_reset(cache->layout_arena);
            }

            if (cache->layout_arena && layout_bytes <= 1024 * 1024)
            {
                uint8_t *canvas_mem = img_slab_alloc(engine->global_pool);
                if (canvas_mem)
                {
                    memcpy(canvas_mem, canvas->buf.data, canvas_bytes);

                    img_cell_t *cells_copy = img_arena_alloc_aligned(
                        cache->layout_arena,
                        layout_bytes,
                        16);

                    if (cells_copy)
                    {
                        memcpy(cells_copy, layout->cells, layout_bytes);

                        cache->canvas = *canvas;
                        cache->canvas.buf.data = canvas_mem;
                        cache->canvas.cache_owned = 1;

                        cache->layout.cells = cells_copy;
                        cache->layout.count = layout->count;

                        cache->photo = photo;
                        cache->cell_w = probe.photo_w_px;
                        cache->cell_h = probe.photo_h_px;
                        cache->job_sig = render_sig;
                        cache->pool = engine->global_pool;
                        cache->valid = 1;
                        cache->final_valid = 1;
                    }
                    else
                    {
                        img_slab_recycle(engine->global_pool, canvas_mem);
                    }
                }
            }
        }
    }

    return r;
}

// #include "runtime/job_exec.h"

// #include "api/api_internal.h"
// #include "memory/slab.h"

// img_result_t img_runtime_prepare_render_stage(
//     img_engine_t *engine,
//     img_canvas_t *canvas,
//     img_layout_t *layout,
//     const img_job_t *job,
//     const img_buffer_t *photo,
//     img_arena_t **arena)
// {
//     if (!engine || !canvas || !layout || !job || !photo || !arena)
//         return IMG_ERR_SECURITY;

//     *arena = img_arena_create(1 * 1024 * 1024);
//     if (!*arena)
//         return IMG_ERR_NOMEM;

//     img_result_t r = img_canvas_init(canvas, engine->global_pool, job);
//     if (r != IMG_SUCCESS)
//     {
//         if (photo->data)
//             img_slab_free(engine->global_pool, (void *)photo->data);
//         return r;
//     }

//     r = img_layout_grid(canvas, photo, job, layout, *arena, engine->global_pool);
//     if (photo->data)
//         img_slab_free(engine->global_pool, (void *)photo->data);

//     return r;
// }
