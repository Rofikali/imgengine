// ./src/api/api_shutdown.c
#include "api/v1/img_api.h"
#include "api/api_internal.h"
#include "api/api_shutdown_internal.h"

void img_api_shutdown(img_engine_t *engine) {
    if (!engine)
        return;

    img_api_shutdown_workers(engine);
    img_api_release_thread_ctx();
    img_api_shutdown_globals(engine);
}
