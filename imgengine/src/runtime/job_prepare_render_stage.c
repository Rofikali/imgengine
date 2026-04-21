#include "runtime/job_exec.h"

#include "api/api_internal.h"
#include "memory/slab.h"

img_result_t img_runtime_prepare_render_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_layout_t *layout,
    const img_job_t *job,
    const img_buffer_t *photo,
    img_arena_t **arena)
{
    if (!engine || !canvas || !layout || !job || !photo || !arena)
        return IMG_ERR_SECURITY;

    *arena = img_arena_create(1 * 1024 * 1024);
    if (!*arena)
        return IMG_ERR_NOMEM;

    img_result_t r = img_canvas_init(canvas, engine->global_pool, job);
    if (r != IMG_SUCCESS)
    {
        if (photo->data)
            img_slab_free(engine->global_pool, (void *)photo->data);
        return r;
    }

    r = img_layout_grid(canvas, photo, job, layout, *arena, engine->global_pool);
    if (photo->data)
        img_slab_free(engine->global_pool, (void *)photo->data);

    return r;
}
