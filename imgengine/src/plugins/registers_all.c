// ./src/plugins/registers_all.c

#include "imgengine/plugins/plugin_registry.h"

/* Match the exact names defined in your .c files */
extern img_plugin_t *get_bleed_plugin();
extern img_plugin_t *get_crop_plugin();

void register_all_plugins(img_ctx_t *ctx) {
    ctx->plugin_count = 0; /* Reset count for new context */
    plugin_register(ctx, get_bleed_plugin());
    plugin_register(ctx, get_crop_plugin());
}
