// ./src/api/api_job_finish_stage.c
#include "api/api_job_internal.h"
#include "runtime/job_exec.h"

img_result_t img_api_finish_job_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_arena_t *arena)
{
    return img_runtime_finish_job_stage(engine, canvas, arena);
}
