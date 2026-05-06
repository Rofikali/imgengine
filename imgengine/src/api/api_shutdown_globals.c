// ./src/api/api_shutdown_globals.c
#include "api/api_shutdown_internal.h"

#include "memory/slab.h"
#include "runtime/template_registry.h"

#include <string.h>

void img_api_shutdown_globals(img_engine_t *engine) {
    if (!engine)
        return;

    img_mpmc_destroy(&g_task_queue);
    memset(&g_task_queue, 0, sizeof(g_task_queue));

    if (engine->user_data) {
        img_template_registry_destroy((img_template_registry_t *)engine->user_data);
        engine->user_data = NULL;
    }

    if (engine->global_pool)
        img_slab_destroy(engine->global_pool);

    engine->global_pool = NULL;
    engine->worker_count = 0;

    memset(g_workers, 0, sizeof(g_workers));
    memset(g_worker_ctxs, 0, sizeof(g_worker_ctxs));
    memset(&g_engine, 0, sizeof(g_engine));
}
