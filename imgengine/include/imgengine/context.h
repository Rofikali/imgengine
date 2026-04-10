// ./include/imgengine/context.h


#ifndef IMG_CONTEXT_H
#define IMG_CONTEXT_H

#include "imgengine/memory_pool.h"

#define MAX_PLUGINS 16

/* Forward declarations for ABI safety */
struct img_layout_info;
struct img_plugin;

typedef struct img_ctx {
    mem_pool_t pool;
    struct img_layout_info *layout;
    struct img_plugin *plugins[MAX_PLUGINS];
    int plugin_count;
} img_ctx_t;

void img_ctx_init(img_ctx_t *ctx, size_t pool_size);
void img_ctx_destroy(img_ctx_t *ctx);

#endif
