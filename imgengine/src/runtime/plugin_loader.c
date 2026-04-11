// ./src/runtime/plugin_loader.c

// ./src/runtime/plugin_loader.c

#include "api/v1/img_plugin_api.h"
#include "runtime/plugin_loader.h"
#include "pipeline/jump_table.h"

// #include <dlfcn.h>
#include <stdio.h>
// #include <dirent.h>
// #include <string.h>

extern const img_plugin_descriptor_t *__start_img_plugins[];
extern const img_plugin_descriptor_t *__stop_img_plugins[];

int img_plugin_load_all(const char *directory)
{
    const img_plugin_descriptor_t **it;

    for (it = __start_img_plugins; it < __stop_img_plugins; ++it)
    {
        const img_plugin_descriptor_t *desc = *it;

        if (desc->abi_version != IMG_PLUGIN_ABI_VERSION)
        {
            printf("ABI mismatch: %s\n", desc->name);
            continue;
        }

        img_register_op(
            desc->op_code,
            desc->single_exec,
            desc->batch_exec);

        printf("Registered plugin: %s\n", desc->name);
    }

    return 0;
}
