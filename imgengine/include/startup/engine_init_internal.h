// ./include/startup/engine_init_internal.h
#ifndef IMGENGINE_STARTUP_ENGINE_INIT_INTERNAL_H
#define IMGENGINE_STARTUP_ENGINE_INIT_INTERNAL_H

#include "core/context_internal.h"
#include "arch/cpu_caps.h"

void img_engine_init_reset(img_engine_t *engine);
void img_engine_init_sandbox(void);
void img_engine_init_caps(img_engine_t *engine);
void img_engine_init_compute(cpu_caps_t caps);
void img_engine_init_plugins_and_io(void);
void img_engine_init_subsystems(img_engine_t *engine);

#endif /* IMGENGINE_STARTUP_ENGINE_INIT_INTERNAL_H */
