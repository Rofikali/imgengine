#ifndef IMGENGINE_RUNTIME_JOB_EXEC_H
#define IMGENGINE_RUNTIME_JOB_EXEC_H

#include "core/result.h"
#include "memory/arena.h"
#include "pipeline/canvas.h"
#include "pipeline/job.h"
#include "pipeline/layout.h"
#include "pipeline/pipeline_types.h"

typedef struct img_engine img_engine_t;
typedef struct img_buffer img_buffer_t;

img_result_t img_runtime_prepare_render_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_layout_t *layout,
    const img_job_t *job,
    const img_buffer_t *photo,
    img_arena_t **arena);

img_result_t img_runtime_apply_job_postfx(
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job);

img_result_t img_runtime_run_compiled_pipeline(
    img_engine_t *engine,
    img_pipeline_desc_t *pipe,
    img_buffer_t *out_buf);

img_result_t img_runtime_finish_job_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_arena_t *arena);

void img_runtime_release_raw_buffer(
    img_engine_t *engine,
    img_buffer_t *buf);

#endif /* IMGENGINE_RUNTIME_JOB_EXEC_H */
