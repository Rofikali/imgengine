// // src/core/context.c

#include "core/context_internal.h"
#include "memory/slab.h"
#include "memory/arena.h"
#include <stdlib.h>

// 🔥 Forward declarations ONLY
// typedef struct img_ctx img_ctx_t;
// typedef struct img_buffer img_buffer_t;
// typedef struct img_batch img_batch_t;

#define DEFAULT_SLAB_SIZE (64 * 1024 * 1024) // 64MB
#define DEFAULT_BLOCK_SIZE (256 * 1024)      // 256KB

// ============================================
// CREATE CONTEXT (PER WORKER)
// ============================================

img_ctx_t *img_ctx_create(uint32_t thread_id, cpu_caps_t caps)
{
    img_ctx_t *ctx = aligned_alloc(64, sizeof(img_ctx_t));
    if (!ctx)
        return NULL;

    ctx->thread_id = thread_id;
    ctx->caps = caps;

    // NUMA-local slab
    ctx->local_pool = img_slab_create(
        DEFAULT_SLAB_SIZE,
        DEFAULT_BLOCK_SIZE);

    if (!ctx->local_pool)
    {
        free(ctx);
        return NULL;
    }

    // ✅ FIXED: correct allocator
    ctx->scratch_arena = img_arena_create(1024 * 1024); // 1MB

    if (!ctx->scratch_arena)
    {
        img_slab_destroy(ctx->local_pool);
        free(ctx);
        return NULL;
    }

    return ctx;
}

// ============================================
// DESTROY CONTEXT
// ============================================

void img_ctx_destroy(img_ctx_t *ctx)
{
    if (!ctx)
        return;

    if (ctx->scratch_arena)
        img_arena_destroy(ctx->scratch_arena); // ✅ FIXED

    if (ctx->local_pool)
        img_slab_destroy(ctx->local_pool);

    free(ctx);
}
