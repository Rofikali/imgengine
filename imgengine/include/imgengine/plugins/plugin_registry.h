// ./include/imgengine/plugins/plugin_registry.h

#ifndef IMG_PLUGIN_REGISTRY_H
#define IMG_PLUGIN_REGISTRY_H

#include "imgengine/context.h"
#include "imgengine/plugins/plugin.h"

/* Registry Management: Context-aware and thread-safe */
void plugin_register(img_ctx_t *ctx, img_plugin_t *plugin);
void register_all_plugins(img_ctx_t *ctx);

#endif
