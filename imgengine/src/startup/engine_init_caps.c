// ./src/startup/engine_init_caps.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"

void img_engine_init_caps(img_engine_t *engine) { engine->caps = img_cpu_detect_caps(); }
