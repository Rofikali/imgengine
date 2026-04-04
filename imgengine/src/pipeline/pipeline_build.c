// src/pipeline/pipeline_build.c

#include "pipeline/pipeline.h"
#include "memory/arena.h"

int img_pipeline_build(
    const img_pipeline_desc_t *desc,
    img_pipeline_runtime_t *runtime,
    img_arena_t *arena)
{
    if (!desc || !runtime || !arena)
        return -1;

    runtime->count = desc->count;

    runtime->ops = (img_op_desc_t *)
        img_arena_alloc(arena,
                        sizeof(img_op_desc_t) * desc->count);

    if (!runtime->ops)
        return -1;

    // 🔥 LINEAR COPY (cache friendly)
    for (uint32_t i = 0; i < desc->count; i++)
    {
        runtime->ops[i] = desc->ops[i];
    }

    return 0;
}