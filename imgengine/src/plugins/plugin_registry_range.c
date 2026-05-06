// ./src/plugins/plugin_registry_range.c
#include "plugins/plugin_registry_internal.h"

void img_plugins_register_range(const img_plugin_descriptor_t *const *begin,
                                const img_plugin_descriptor_t *const *end) {
    for (const img_plugin_descriptor_t *const *it = begin; it < end; ++it) {
        const img_plugin_descriptor_t *p = *it;
        if (img_plugin_descriptor_is_valid(p))
            img_plugin_descriptor_register(p);
    }
}
