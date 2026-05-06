// ./src/core/batch_builder.c
// src/core/batch_builder.c  (UPDATE)
// Remove pipeline/batch_exec.h — use forward decls only

#define _GNU_SOURCE

#include "core/batch.h"
#include "memory/arena.h"
#include "core/context_internal.h"

img_batch_t *img_batch_create(img_ctx_t *ctx, uint32_t count) {
    img_batch_t *batch = img_arena_alloc(ctx->scratch_arena, sizeof(img_batch_t));

    if (!batch)
        return NULL;
    batch->count = count;
    return batch;
}

// // ./src/core/batch_builder.c

// #include "pipeline/batch_exec.h"
// #include "memory/arena.h"

// img_batch_t *img_batch_create(
//     img_ctx_t *ctx,
//     uint32_t count)
// {
//     img_batch_t *batch =
//         img_arena_alloc(ctx->scratch_arena,
//                         sizeof(img_batch_t));

//     batch->count = count;

//     return batch;
// }
