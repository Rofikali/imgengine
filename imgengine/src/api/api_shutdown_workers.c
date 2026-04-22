// ./src/api/api_shutdown_workers.c
#include "api/api_shutdown_internal.h"

#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "pipeline/render_cache.h"

extern img_scheduler_t g_scheduler;

void img_api_shutdown_workers(img_engine_t *engine)
{
    if (!engine)
        return;

    if (g_scheduler.workers)
    {
        img_scheduler_destroy(&g_scheduler);
        engine->workers = NULL;
        engine->scheduler = NULL;
        return;
    }

    for (uint32_t i = 0; i < engine->worker_count; i++)
    {
        img_worker_stop(&engine->workers[i]);
        img_worker_join(&engine->workers[i]);
        img_render_cache_discard(&engine->workers[i].render_cache);
        if (engine->workers[i].queue)
        {
            img_queue_destroy(engine->workers[i].queue);
            engine->workers[i].queue = NULL;
        }
    }
}
