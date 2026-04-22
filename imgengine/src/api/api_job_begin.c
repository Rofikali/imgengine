// ./src/api/api_job_begin.c
#include "api/api_job_internal.h"
#include "runtime/job_exec.h"
#include "runtime/scheduler.h"
#include "runtime/task.h"

img_result_t img_api_begin_job(
    img_engine_t *engine,
    const char *input_path,
    const img_job_t *job,
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    img_layout_t *layout,
    img_arena_t **arena)
{
    if (!engine || !input_path || !job || !ctx || !canvas || !layout || !arena)
        return IMG_ERR_SECURITY;

    uint8_t *file_data = NULL;
    size_t file_size = 0;
    img_buffer_t photo = {0};

    img_result_t r = img_api_init_ctx(engine, ctx);
    if (r != IMG_SUCCESS)
        return r;

    r = img_api_load_input(input_path, &file_data, &file_size);
    if (r != IMG_SUCCESS)
        return r;

    r = img_api_decode_input(ctx, &file_data, file_size, &photo);
    if (r != IMG_SUCCESS)
        return r;

    if (engine->scheduler && engine->worker_count > 0)
    {
        img_task_t task = {
            .kind = IMG_TASK_KIND_RENDER_STAGE,
            .state = IMG_TASK_READY,
            .status = IMG_ERR_INTERNAL,
            .payload.render = {
                .engine = engine,
                .ctx = ctx,
                .canvas = canvas,
                .layout = layout,
                .job = job,
                .photo = &photo,
                .arena = arena,
            },
        };

        if (img_runtime_submit_task(engine, &task) == 0)
            r = img_runtime_wait_task(&task);
        else
            r = img_api_prepare_render_stage(
                engine, ctx, canvas, layout, job, &photo, arena);
    }
    else
    {
        r = img_api_prepare_render_stage(
            engine, ctx, canvas, layout, job, &photo, arena);
    }

    img_runtime_release_raw_buffer(engine, &photo);
    return r;
}
