#include "api/v1/img_plugin_api.h"
#include "pipeline/dispatch/jump_table.h"

#include <stdio.h>

/*
 * 🔥 LINKER SYMBOLS (AUTO-GENERATED)
 */
extern const img_plugin_descriptor_t *__start_img_plugins;
extern const img_plugin_descriptor_t *__stop_img_plugins;

void img_plugins_init_all(void)
{
    const img_plugin_descriptor_t **it = &__start_img_plugins;
    const img_plugin_descriptor_t **end = &__stop_img_plugins;

    for (; it < end; ++it)
    {
        const img_plugin_descriptor_t *p = *it;

        if (!p) continue;

        /*
         * 🔥 ABI SAFETY
         */
        if (p->abi_version != IMG_PLUGIN_ABI_VERSION)
        {
            fprintf(stderr, "❌ Plugin ABI mismatch: %s\n", p->name);
            continue;
        }

        /*
         * 🔥 REGISTER INTO JUMP TABLE
         */
        img_register_op(
            p->op_code,
            p->single_exec);

        /*
         * 🔥 OPTIONAL: batch path later
         */
    }
}