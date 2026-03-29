// src/plugins/register_all.c

// #include "imgengine/plugins/plugin_registry.h"

// img_plugin_t *get_bleed_plugin();
// img_plugin_t *get_crop_plugin();

// void register_all_plugins()
// {
//     plugin_register(get_bleed_plugin());
//     plugin_register(get_crop_plugin());
//     // register_plugin(get_bleed_plugin());
//     // register_plugin(get_crop_plugin());
// }


#include "imgengine/plugins/plugin_registry.h"

/* Match the exact names defined in your .c files */
extern img_plugin_t *get_bleed_plugin();
extern img_plugin_t *get_crop_plugin(); 

void register_all_plugins(img_ctx_t *ctx)
{
    ctx->plugin_count = 0; /* Reset count for new context */
    plugin_register(ctx, get_bleed_plugin());
    plugin_register(ctx, get_crop_plugin());
}



// #include "imgengine/plugins/plugin_registry.h"

// /* Extern getters for individual plugin implementations */
// extern img_plugin_t *get_bleed_plugin();
// extern img_plugin_t *get_crop_marks_plugin();

// void register_all_plugins(img_ctx_t *ctx)
// {
//     plugin_register(ctx, get_bleed_plugin());
//     plugin_register(ctx, get_crop_marks_plugin());
// }
