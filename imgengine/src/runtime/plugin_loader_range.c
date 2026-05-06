// ./src/runtime/plugin_loader_range.c
#include "runtime/plugin_loader_internal.h"

int img_plugin_descriptor_load_range(const img_plugin_descriptor_t *const *begin,
                                     const img_plugin_descriptor_t *const *end) {
    for (const img_plugin_descriptor_t *const *it = begin; it < end; ++it) {
        if (img_plugin_descriptor_load(*it) != 0)
            continue;
    }

    return 0;
}
