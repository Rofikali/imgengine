#ifndef IMGENGINE_API_IMGENGINE_ENGINE_PRIVATE_H
#define IMGENGINE_API_IMGENGINE_ENGINE_PRIVATE_H

#include <pthread.h>

#include "api/v1/img_api.h"
#include "imgengine/api/v1/imgengine.h"

struct imgengine_engine {
    img_engine_t *native_engine;
    pthread_mutex_t operation_lock;
};

#endif /* IMGENGINE_API_IMGENGINE_ENGINE_PRIVATE_H */
