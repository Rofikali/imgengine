// // plugins/plugin_registry.c

#include "api/v1/img_plugin_api.h"
#include "pipeline/jump_table.h"
#include <stdio.h>

/*
 * 🔥 LINKER-DEFINED RANGE
 */
extern const img_plugin_descriptor_t *__start_img_plugins;
extern const img_plugin_descriptor_t *__stop_img_plugins;

/*
 * 🔥 INIT ALL (ZERO RUNTIME REGISTRATION)
 */
void img_plugins_init_all(void)
{
    const img_plugin_descriptor_t **it =
        &__start_img_plugins;

    const img_plugin_descriptor_t **end =
        &__stop_img_plugins;

    for (; it < end; ++it)
    {
        const img_plugin_descriptor_t *p = *it;

        if (!p)
            continue;

        if (p->abi_version != IMG_PLUGIN_ABI_VERSION)
        {
            fprintf(stderr, "ABI mismatch: %s\n", p->name);
            continue;
        }

        img_register_op(
            p->op_code,
            (p->capabilities & IMG_CAP_SINGLE) ? p->single_exec : NULL,
            (p->capabilities & IMG_CAP_BATCH) ? p->batch_exec : NULL);
    }
}

// #include "api/v1/img_plugin_api.h"
// #include "pipeline/jump_table.h"
// #include "core/opcodes.h"

// #include <dlfcn.h>
// #include <stdio.h>

// #define MAX_PLUGINS 128

// static const img_plugin_descriptor_t *g_plugins[MAX_PLUGINS];
// static uint32_t g_plugin_count = 0;

// static void register_plugin(const img_plugin_descriptor_t *p)
// {
//     if (!p)
//         return;

//     if (p->abi_version != IMG_PLUGIN_ABI_VERSION)
//     {
//         fprintf(stderr, "ABI mismatch for plugin: %s\n", p->name);
//         return;
//     }

//     if (g_plugin_count >= MAX_PLUGINS)
//         return;

//     g_plugins[g_plugin_count++] = p;

//     img_op_fn single = NULL;
//     img_batch_op_fn batch = NULL;

//     if (p->capabilities & IMG_CAP_SINGLE)
//         single = p->single_exec;

//     if (p->capabilities & IMG_CAP_BATCH)
//         batch = p->batch_exec;

//     /*
//      * 🔥 SINGLE UNIFIED REGISTRATION (L10 STYLE)
//      */
//     img_register_op(p->op_code, single, batch);
// }
