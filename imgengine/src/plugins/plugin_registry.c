// ./src/plugins/plugin_registry.c

#include "plugins/plugin_registry_internal.h"

extern const img_plugin_descriptor_t *__start_img_plugins[];
extern const img_plugin_descriptor_t *__stop_img_plugins[];

void img_plugins_init_all(void) {
    img_plugins_register_range(__start_img_plugins, __stop_img_plugins);
}
