
// // src/core/context.c

#include "core/context_internal.h"
#include "memory/slab.h"
#include "memory/arena.h"
#include <stdlib.h>

#define DEFAULT_SLAB_SIZE (64 * 1024 * 1024)
#define DEFAULT_BLOCK_SIZE (256 * 1024)

img_ctx_t *img_ctx_create(uint32_t thread_id, cpu_caps_t caps)
{
    img_ctx_t *ctx = aligned_alloc(64, sizeof(img_ctx_t));
    if (!ctx)
        return NULL;

    ctx->thread_id = thread_id;
    ctx->caps = caps;

    ctx->local_pool = img_slab_create(
        DEFAULT_SLAB_SIZE,
        DEFAULT_BLOCK_SIZE);

    if (!ctx->local_pool)
        goto fail;

    ctx->scratch_arena = img_arena_create(1024 * 1024);
    if (!ctx->scratch_arena)
        goto fail_pool;

    return ctx;

fail_pool:
    img_slab_destroy(ctx->local_pool);
fail:
    free(ctx);
    return NULL;
}

void img_ctx_destroy(img_ctx_t *ctx)
{
    if (!ctx)
        return;

    img_arena_destroy(ctx->scratch_arena);
    img_slab_destroy(ctx->local_pool);

    free(ctx);
}
