// ./src/runtime/benchmark_hot.c
#include "runtime/benchmark_hot.h"
#include "runtime/job_exec.h"
#include "runtime/template_registry.h"

#include <string.h>
#include <stdlib.h>

img_result_t img_runtime_hot_bench_init(img_engine_t *engine, const img_buffer_t *decoded,
                                        img_job_template_t preset_template,
                                        img_hot_bench_state_t *state) {
    if (!engine || !decoded || !decoded->data || !state)
        return IMG_ERR_SECURITY;

    memset(state, 0, sizeof(*state));
    state->engine = engine;
    state->decoded = *decoded;
    state->owns_decoded = 0;

    if (state->decoded.width == 0 || state->decoded.height == 0) {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_FORMAT;
    }

    img_ctx_bind_engine(engine, &state->ctx);
    state->arena = img_arena_create(1 * 1024 * 1024);
    if (!state->arena) {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_NOMEM;
    }

    state->ctx.scratch_arena = state->arena;
    state->render_cache.pool = engine->global_pool;
    state->render_cache.allow_scaled_cache = 1;
    /*
     * Keep scaled-cache reuse enabled. By default disable the final canvas
     * cache for benchmarking to avoid measuring warm-cache hits. Allow
     * overriding via the environment variable `IMGENGINE_ENABLE_FINAL_CACHE`
     * so we can measure the effect of final-buffer reuse as an immediate
     * optimization (quick win).
     */
    if (getenv("IMGENGINE_ENABLE_FINAL_CACHE") != NULL)
        state->render_cache.allow_final_cache = 1;
    else
        state->render_cache.allow_final_cache = 0;
    state->ctx.op_params = &state->render_cache;
    if (img_runtime_resolve_template_job(engine, preset_template, &state->job) != IMG_SUCCESS) {
        img_job_defaults(&state->job);
        state->job.cols = 6;
        state->job.rows = 3;
        state->job.gap = 20;
        state->job.padding = 20;
    }

    return IMG_SUCCESS;
}

img_result_t img_runtime_hot_bench_step(img_hot_bench_state_t *state) {
    if (!state || !state->engine || !state->decoded.data)
        return IMG_ERR_SECURITY;

    return img_runtime_prepare_render_stage(state->engine, &state->ctx, &state->canvas,
                                            &state->layout, &state->job, &state->decoded,
                                            &state->arena);
}

void img_runtime_hot_bench_destroy(img_engine_t *engine, img_hot_bench_state_t *state) {
    if (!state)
        return;

    if (engine && state->canvas.buf.data)
        img_canvas_release(&state->canvas, engine->global_pool);

    img_render_cache_discard(&state->render_cache);

    if (state->arena)
        img_arena_destroy(state->arena);

    if (engine && state->owns_decoded && state->decoded.data)
        img_runtime_release_raw_buffer(engine, &state->decoded);

    memset(state, 0, sizeof(*state));
}
