#include "runtime/benchmark_hot.h"
#include "runtime/job_exec.h"
#include "runtime/scheduler.h"
#include "runtime/task.h"

#include <string.h>

img_result_t img_runtime_hot_bench_init(
    img_engine_t *engine,
    const img_buffer_t *decoded,
    img_job_template_t preset_template,
    img_hot_bench_state_t *state)
{
    if (!engine || !decoded || !decoded->data || !state)
        return IMG_ERR_SECURITY;

    memset(state, 0, sizeof(*state));
    state->engine = engine;
    state->decoded = *decoded;

    if (state->decoded.width == 0 || state->decoded.height == 0)
    {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_FORMAT;
    }

    img_ctx_bind_engine(engine, &state->ctx);
    state->render_cache.pool = engine->global_pool;
    /*
     * Benchmark must measure the real render path, not the render-cache
     * shortcut. Keep the cache object around for destruction symmetry,
     * but do not wire it into ctx->op_params here.
     */
    state->ctx.op_params = NULL;
    img_job_defaults(&state->job);
    if (preset_template != IMG_JOB_TEMPLATE_CUSTOM)
    {
        img_job_apply_template(&state->job, preset_template);
    }
    else
    {
        state->job.cols = 6;
        state->job.rows = 3;
        state->job.gap = 20;
        state->job.padding = 20;
    }

    return IMG_SUCCESS;
}

img_result_t img_runtime_hot_bench_step(
    img_hot_bench_state_t *state)
{
    if (!state || !state->engine || !state->decoded.data)
        return IMG_ERR_SECURITY;

    if (state->engine->scheduler && state->engine->worker_count > 0)
    {
        img_task_t task = {
            .kind = IMG_TASK_KIND_RENDER_STAGE,
            .state = IMG_TASK_READY,
            .status = IMG_ERR_INTERNAL,
            .payload.render = {
                .engine = state->engine,
                .ctx = &state->ctx,
                .canvas = &state->canvas,
                .layout = &state->layout,
                .job = &state->job,
                .photo = &state->decoded,
                .arena = &state->arena,
            },
        };

        if (img_runtime_submit_task(state->engine, &task) == 0)
            return img_runtime_wait_task(&task);
    }

    return img_runtime_prepare_render_stage(
        state->engine,
        &state->ctx,
        &state->canvas,
        &state->layout,
        &state->job,
        &state->decoded,
        &state->arena);
}

void img_runtime_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state)
{
    if (!state)
        return;

    if (engine && state->canvas.buf.data)
        img_canvas_release(&state->canvas, engine->global_pool);

    img_render_cache_discard(&state->render_cache);

    if (state->arena)
        img_arena_destroy(state->arena);

    if (engine && state->decoded.data)
        img_runtime_release_raw_buffer(engine, &state->decoded);

    memset(state, 0, sizeof(*state));
}
