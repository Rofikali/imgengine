// ./src/core/context_destroy.c
#include "core/context_internal.h"
#include "memory/slab.h"
#include "memory/arena.h"

#include <stdlib.h>

void img_ctx_destroy(img_ctx_t *ctx) {
    if (!ctx)
        return;

    img_arena_destroy(ctx->scratch_arena);
    img_slab_destroy(ctx->local_pool);

    free(ctx);
}
