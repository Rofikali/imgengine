// ./src/plugins/plugin_registry_register.c
#include "plugins/plugin_registry_internal.h"
#include "pipeline/jump_table.h"

#include <stddef.h>

void img_plugin_descriptor_register(const img_plugin_descriptor_t *p) {
    img_register_op(p->op_code, (p->capabilities & IMG_CAP_SINGLE) ? p->single_exec : NULL,
                    (p->capabilities & IMG_CAP_BATCH) ? p->batch_exec : NULL);
}
