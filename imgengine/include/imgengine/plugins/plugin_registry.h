// imgengine/plugins/plugin_register.h

// #ifndef IMG_PLUGIN_REGISTRY_H
// #define IMG_PLUGIN_REGISTRY_H

// #include "plugin.h"

// void plugin_register(img_plugin_t *plugin);
// void register_all_plugins();
// img_plugin_t **plugin_get_all(int *count);

// #endif

#ifndef IMG_PLUGIN_REGISTRY_H
#define IMG_PLUGIN_REGISTRY_H

#include "imgengine/plugins/plugin.h"
#include "imgengine/context.h"

/* Registry Management: Context-aware and thread-safe */
void plugin_register(img_ctx_t *ctx, img_plugin_t *plugin);
void register_all_plugins(img_ctx_t *ctx);

#endif




// #ifndef IMG_PLUGIN_REGISTRY_H
// #define IMG_PLUGIN_REGISTRY_H

// #include "imgengine/context.h"

// typedef struct img_plugin
// {
//     const char *name;
//     int (*should_run)(const struct img_job *job);
//     int (*process)(img_ctx_t *ctx, img_t *canvas, const struct img_job *job);
// } img_plugin_t;

// void plugin_register(img_ctx_t *ctx, img_plugin_t *plugin);
// void register_all_plugins(img_ctx_t *ctx);

// #endif
