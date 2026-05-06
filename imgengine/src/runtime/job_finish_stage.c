// ./src/runtime/job_finish_stage.c
#include "runtime/job_exec.h"
#include "core/context_internal.h"

img_result_t img_runtime_finish_job_stage(img_engine_t *engine, img_canvas_t *canvas,
                                          img_arena_t *arena) {
    if (!engine || !canvas)
        return IMG_ERR_SECURITY;

    if (canvas->buf.data)
        img_canvas_release(canvas, engine->global_pool);

    if (arena)
        img_arena_destroy(arena);

    return IMG_SUCCESS;
}
