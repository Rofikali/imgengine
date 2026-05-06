// ./src/runtime/plugin_loader_descriptor.c
#include "runtime/plugin_loader_internal.h"
#include "pipeline/jump_table.h"

#include <stdio.h>

int img_plugin_descriptor_load(const img_plugin_descriptor_t *desc) {
    if (!desc)
        return -1;

    if (desc->abi_version != IMG_PLUGIN_ABI_VERSION) {
        printf("ABI mismatch: %s\n", desc->name);
        return -1;
    }

    img_register_op(desc->op_code, desc->single_exec, desc->batch_exec);

    printf("Registered plugin: %s\n", desc->name);
    return 0;
}
