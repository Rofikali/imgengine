// ./src/startup/engine_init.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"

void img_engine_init(img_engine_t *engine) {
    if (!engine)
        return;

    img_engine_init_reset(engine);
    img_engine_init_sandbox();
    img_engine_init_subsystems(engine);
}
