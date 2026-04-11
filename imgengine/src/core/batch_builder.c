// ./src/core/batch_builder.c

// src/core/batch_builder.c

#include "pipeline/batch_exec.h"
#include "memory/arena.h"

img_batch_t *img_batch_create(
    img_ctx_t *ctx,
    uint32_t count)
{
    img_batch_t *batch =
        img_arena_alloc(ctx->scratch_arena,
                        sizeof(img_batch_t));

    batch->count = count;

    return batch;
}