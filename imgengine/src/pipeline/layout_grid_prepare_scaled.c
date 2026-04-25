
// ================================================================
// FILE 8: src/pipeline/layout_grid_prepare_scaled.c  (UPDATE)
// Thread ctx through.
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout_grid_internal.h"

img_result_t img_layout_grid_prepare_scaled(
    img_ctx_t *ctx,
    const img_buffer_t *photo,
    const img_job_t *job,
    uint32_t pw,
    uint32_t ph,
    img_slab_pool_t *pool,
    img_buffer_t *scaled,
    uint8_t *owned_out)
{
    img_render_cache_t *cache = ctx ? (img_render_cache_t *)ctx->op_params : NULL;
    uint32_t sig = img_render_cache_signature(photo, job, pw, ph);

    if (cache && cache->allow_scaled_cache &&
        cache->valid &&
        cache->photo == photo &&
        cache->cell_w == pw &&
        cache->cell_h == ph &&
        cache->job_sig == sig &&
        cache->scaled.data)
    {
        *scaled = cache->scaled;
        if (owned_out)
            *owned_out = 0;
        return IMG_SUCCESS;
    }

    if (job->mode == IMG_FIT)
    {
        img_result_t r = img_layout_prepare_fit(
            ctx, photo, scaled, pw, ph, pool,
            job->bg_r, job->bg_g, job->bg_b);
        if (r != IMG_SUCCESS)
            return r;

        if (cache && cache->allow_scaled_cache)
        {
            if (cache->valid)
                img_render_cache_discard(cache);

            cache->scaled = *scaled;
            cache->photo = photo;
            cache->cell_w = pw;
            cache->cell_h = ph;
            cache->job_sig = sig;
            cache->pool = pool;
            cache->valid = 1;
            if (owned_out)
                *owned_out = 0;
        }
        else if (owned_out)
        {
            *owned_out = 1;
        }
        return r;
    }

    img_result_t r = img_layout_prepare_fill(ctx, photo, scaled, pw, ph, pool);
    if (r != IMG_SUCCESS)
        return r;

    if (cache && cache->allow_scaled_cache)
    {
        if (cache->valid)
            img_render_cache_discard(cache);

        cache->scaled = *scaled;
        cache->photo = photo;
        cache->cell_w = pw;
        cache->cell_h = ph;
        cache->job_sig = sig;
        cache->pool = pool;
        cache->valid = 1;
        if (owned_out)
            *owned_out = 0;
    }
    else if (owned_out)
    {
        *owned_out = 1;
    }

    return r;
}
