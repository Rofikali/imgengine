// ./include/pipeline/render_cache.h
#ifndef IMGENGINE_PIPELINE_RENDER_CACHE_H
#define IMGENGINE_PIPELINE_RENDER_CACHE_H

#include <stdint.h>

#include "core/buffer.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include "pipeline/canvas.h"
#include "pipeline/layout.h"
#include "pipeline/job.h"

typedef struct img_render_cache {
    img_buffer_t scaled;
    img_canvas_t canvas;
    img_layout_t layout;

    const img_buffer_t *photo;
    uint32_t cell_w;
    uint32_t cell_h;
    uint32_t job_sig;

    img_slab_pool_t *pool;
    img_arena_t *layout_arena;

    uint8_t valid;
    uint8_t final_valid;
    uint8_t allow_scaled_cache;
    uint8_t allow_final_cache;
} img_render_cache_t;

static inline uint32_t img_render_cache_mix_u32(uint32_t sig, uint32_t v) {
    sig ^= v + 0x9e3779b9u + (sig << 6) + (sig >> 2);
    return sig;
}

static inline uint32_t img_render_cache_bits_f32(float value) {
    union {
        float f;
        uint32_t u;
    } v = {.f = value};
    return v.u;
}

static inline uint32_t img_render_cache_signature(const img_buffer_t *photo, const img_job_t *job,
                                                  uint32_t cell_w, uint32_t cell_h) {
    uintptr_t p = (uintptr_t)(photo ? photo->data : NULL);
    uint32_t sig = (uint32_t)(p ^ (p >> 32));

    if (photo) {
        sig ^= photo->width * 16777619u;
        sig ^= photo->height * 2166136261u;
        sig ^= photo->channels * 2654435761u;
        sig ^= photo->stride * 40503u;
    }

    if (job) {
        sig = img_render_cache_mix_u32(sig, job->template_id);
        sig = img_render_cache_mix_u32(sig, job->abi_version);
        sig = img_render_cache_mix_u32(sig, img_render_cache_bits_f32(job->photo_w_cm));
        sig = img_render_cache_mix_u32(sig, img_render_cache_bits_f32(job->photo_h_cm));
        sig = img_render_cache_mix_u32(sig, job->dpi);
        sig = img_render_cache_mix_u32(sig, job->cols);
        sig = img_render_cache_mix_u32(sig, job->rows);
        sig = img_render_cache_mix_u32(sig, job->gap);
        sig = img_render_cache_mix_u32(sig, job->padding);
        sig = img_render_cache_mix_u32(sig, job->border_px);
        sig = img_render_cache_mix_u32(sig, job->bleed_px);
        sig = img_render_cache_mix_u32(sig, job->crop_mark_px);
        sig = img_render_cache_mix_u32(sig, job->crop_thickness);
        sig = img_render_cache_mix_u32(sig, job->crop_offset_px);
        sig = img_render_cache_mix_u32(sig, job->mode);
        sig = img_render_cache_mix_u32(sig, ((uint32_t)job->bg_r << 16) |
                                                ((uint32_t)job->bg_g << 8) | (uint32_t)job->bg_b);
    }

    sig ^= cell_w * 2246822519u;
    sig ^= cell_h * 3266489917u;
    return sig;
}

static inline uint8_t img_render_cache_match(const img_render_cache_t *cache,
                                             const img_buffer_t *photo, const img_job_t *job,
                                             uint32_t cell_w, uint32_t cell_h) {
    if (!cache || !cache->valid)
        return 0;

    uint32_t sig = img_render_cache_signature(photo, job, cell_w, cell_h);
    return cache->photo == photo && cache->cell_w == cell_w && cache->cell_h == cell_h &&
           cache->job_sig == sig;
}

static inline void img_render_cache_discard(img_render_cache_t *cache) {
    if (!cache)
        return;

    const uint8_t allow_scaled_cache = cache->allow_scaled_cache;
    const uint8_t allow_final_cache = cache->allow_final_cache;

    if (cache->valid && cache->pool && cache->scaled.data)
        img_slab_recycle(cache->pool, cache->scaled.data);

    if (cache->final_valid && cache->pool && cache->canvas.buf.data)
        img_slab_recycle(cache->pool, cache->canvas.buf.data);

    if (cache->layout_arena)
        img_arena_destroy(cache->layout_arena);

    cache->scaled = (img_buffer_t){0};
    cache->canvas = (img_canvas_t){0};
    cache->layout = (img_layout_t){0};
    cache->photo = NULL;
    cache->cell_w = 0;
    cache->cell_h = 0;
    cache->job_sig = 0;
    cache->pool = NULL;
    cache->layout_arena = NULL;
    cache->valid = 0;
    cache->final_valid = 0;
    cache->allow_scaled_cache = allow_scaled_cache;
    cache->allow_final_cache = allow_final_cache;
}

#endif /* IMGENGINE_PIPELINE_RENDER_CACHE_H */
