// ./src/core/context.c
// Legacy coordination unit split into:
//   - src/core/context_create.c
//   - src/core/context_destroy.c

#include "core/context_internal.h"

#include <stddef.h>

void img_ctx_bind_engine(const img_engine_t *engine, img_ctx_t *ctx)
{
    if (!engine || !ctx)
        return;

    ctx->thread_id = 0;
    ctx->caps = engine->caps;
    ctx->local_pool = engine->global_pool;
    ctx->scratch_arena = NULL;
    ctx->op_params = NULL;
    ctx->fused_params = NULL;
}
