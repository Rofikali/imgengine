// ./include/plugins/plugin_registry_internal.h
#ifndef IMGENGINE_PLUGINS_PLUGIN_REGISTRY_INTERNAL_H
#define IMGENGINE_PLUGINS_PLUGIN_REGISTRY_INTERNAL_H

#include "pipeline/plugin_abi.h"
#include <stdbool.h>

bool img_plugin_descriptor_is_valid(const img_plugin_descriptor_t *p);
void img_plugin_descriptor_register(const img_plugin_descriptor_t *p);
void img_plugins_register_range(const img_plugin_descriptor_t *const *begin,
                                const img_plugin_descriptor_t *const *end);

#endif /* IMGENGINE_PLUGINS_PLUGIN_REGISTRY_INTERNAL_H */
