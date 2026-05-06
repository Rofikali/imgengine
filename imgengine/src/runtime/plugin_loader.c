// ./src/runtime/plugin_loader.c

#include "runtime/plugin_loader.h"
#include "runtime/plugin_loader_internal.h"

int img_plugin_load_all(const char *directory) {
    if (!directory)
        return -1;

    extern const img_plugin_descriptor_t *__start_img_plugins[];
    extern const img_plugin_descriptor_t *__stop_img_plugins[];
    img_plugin_descriptor_load_range(__start_img_plugins, __stop_img_plugins);
    return 0;
}
