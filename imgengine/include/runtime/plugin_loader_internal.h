// ./include/runtime/plugin_loader_internal.h
#ifndef IMGENGINE_RUNTIME_PLUGIN_LOADER_INTERNAL_H
#define IMGENGINE_RUNTIME_PLUGIN_LOADER_INTERNAL_H

#include "pipeline/plugin_abi.h"

int img_plugin_descriptor_load(const img_plugin_descriptor_t *desc);
int img_plugin_descriptor_load_range(const img_plugin_descriptor_t *const *begin,
                                     const img_plugin_descriptor_t *const *end);

#endif /* IMGENGINE_RUNTIME_PLUGIN_LOADER_INTERNAL_H */
