// ./src/startup/engine_init_plugins_and_io.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"
#include "runtime/plugin_loader.h"
#include "api/io_vtable.h"

void img_plugins_init_all(void);
void img_io_register_defaults(void);

void img_engine_init_plugins_and_io(void) {
    img_plugins_init_all();
    img_plugin_load_all("./plugins_build");
    img_io_register_defaults();
}
