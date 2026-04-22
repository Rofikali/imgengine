// ./src/runtime/scheduler_destroy.c
#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "pipeline/render_cache.h"

#include <stdlib.h>

void img_scheduler_destroy(img_scheduler_t *sched)
{
    if (!sched)
        return;

    for (uint32_t i = 0; i < sched->worker_count; i++)
    {
        img_worker_stop(&sched->workers[i]);
        img_worker_join(&sched->workers[i]);
        img_render_cache_discard(&sched->workers[i].render_cache);
        if (sched->workers[i].queue)
        {
            img_queue_destroy(sched->workers[i].queue);
            sched->workers[i].queue = NULL;
        }
    }

    img_mpmc_destroy(&sched->global_queue);
    free(sched->workers);
    sched->workers = NULL;
    sched->worker_count = 0;
}
