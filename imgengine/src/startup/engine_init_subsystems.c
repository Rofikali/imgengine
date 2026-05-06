// ./src/startup/engine_init_subsystems.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"
void img_engine_init_subsystems(img_engine_t *engine) {
    img_engine_init_caps(engine);
    img_engine_init_compute(engine->caps);
    img_engine_init_plugins_and_io();
}
