// ./src/api/api_job_prepare_render_stage.c
#include "api/api_job_internal.h"
#include "runtime/job_exec.h"

img_result_t img_api_prepare_render_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_layout_t *layout,
    const img_job_t *job,
    const img_buffer_t *photo,
    img_arena_t **arena)
{
    return img_runtime_prepare_render_stage(
        engine, canvas, layout, job, photo, arena);
}
