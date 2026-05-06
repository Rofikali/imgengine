// ./src/startup/engine_init_reset.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"

#include <string.h>

void img_engine_init_reset(img_engine_t *engine) { memset(engine, 0, sizeof(*engine)); }
