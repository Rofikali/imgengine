// ./src/plugins/plugin_registry.c


#include "imgengine/plugins/plugin_registry.h"
#include <stddef.h>

void plugin_register(img_ctx_t *ctx, img_plugin_t *plugin)
{
    /* Safety: Linux kernel style bounds checking */
    if (ctx && plugin && ctx->plugin_count < MAX_PLUGINS)
    {
        ctx->plugins[ctx->plugin_count++] = plugin;
    }
}


// #include "imgengine/plugins/plugin.h"
// #include "imgengine/plugins/plugin_registry.h"

// #define MAX_PLUGINS 32

// static img_plugin_t *g_plugins[MAX_PLUGINS];
// static int g_plugin_count = 0;

// img_plugin_t **plugin_get_all(int *count)
// {
//     *count = g_plugin_count;
//     return g_plugins;
// }

// void plugin_register(img_ctx_t *ctx, img_plugin_t *plugin)
// {
//     if (ctx->plugin_count < MAX_PLUGINS && plugin != NULL)
//     {
//         ctx->plugins[ctx->plugin_count++] = plugin;
//     }
// }
