// ./src/plugins/plugin_registry.c

// ./src/plugins/plugin_registry.c

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
