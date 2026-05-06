// ./src/plugins/plugin_registry_validate.c
#include "plugins/plugin_registry_internal.h"

bool img_plugin_descriptor_is_valid(const img_plugin_descriptor_t *p) {
    if (!p)
        return false;
    return p->abi_version == IMG_PLUGIN_ABI_VERSION;
}
