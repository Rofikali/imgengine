// ./src/core/context.c

#include "imgengine/context.h"
#include "imgengine/layout.h"
#include "imgengine/plugins/plugin_registry.h"
#include <stdlib.h>
#include <string.h>

void img_ctx_init(img_ctx_t *ctx, size_t pool_size) {
    mp_init(&ctx->pool, pool_size);
    ctx->layout = NULL;
    ctx->plugin_count = 0;

    /* Initialize plugins for THIS context (Thread Safe) */
    register_all_plugins(ctx);
}

IE_API img_layout_info_t *layout_get(img_ctx_t *ctx) {
    /* Kernel-grade safety check */
    if (!ctx)
        return NULL;
    return ctx->layout;
}

void img_ctx_destroy(img_ctx_t *ctx) {
    if (ctx) {
        mp_destroy(&ctx->pool);
    }
}

void layout_store(img_ctx_t *ctx, img_cell_t *cells, int count) {
    /* Slab Allocation: Info + Data in one contiguous block */
    size_t data_size = sizeof(img_cell_t) * count;
    size_t total_size = sizeof(img_layout_info_t) + data_size;

    img_layout_info_t *info = (img_layout_info_t *)mp_alloc(&ctx->pool, total_size);
    if (!info)
        return;

    /* Point cells to the memory immediately following the struct */
    info->cells = (img_cell_t *)(info + 1);
    memcpy(info->cells, cells, data_size);
    info->count = count;

    ctx->layout = info;
}
