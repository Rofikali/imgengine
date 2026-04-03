
// src/cold/debug.c

#include "cold/debug.h"
#include <stdio.h>

void img_debug_dump_ctx(const img_ctx_t *ctx)
{
    if (!ctx)
        return;

    printf("[DEBUG] Worker: %u Pool: %p Caps: %u\n",
           ctx->worker_id, // ✅ FIXED
           (void *)ctx->pool,
           ctx->cpu_caps);
}