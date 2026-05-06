// ./include/api/api_shutdown_internal.h
#ifndef IMGENGINE_API_SHUTDOWN_INTERNAL_H
#define IMGENGINE_API_SHUTDOWN_INTERNAL_H

#include "api/api_internal.h"

void img_api_shutdown_workers(img_engine_t *engine);
void img_api_shutdown_globals(img_engine_t *engine);

#endif /* IMGENGINE_API_SHUTDOWN_INTERNAL_H */
