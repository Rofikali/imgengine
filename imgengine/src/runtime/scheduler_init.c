
// ================================================================
// FILE 2: src/runtime/scheduler_init.c  (REPLACE)
//
// WAS: workers initialized with NULL ctx — they crash on first task
// FIX: ctx is set per-worker from engine's ctx array
//
// NOTE: scheduler_init doesn't have engine access.
// The fix is in api_init_workers.c (File 3) which does.
// Here we just keep scheduler_init clean — no engine dependency.
// Scheduler workers get ctx set externally after init.
// ================================================================

#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include <stdlib.h>

int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)
{
    if (!sched || workers == 0)
        return -1;

    sched->worker_count = workers;
    sched->workers = calloc(workers, sizeof(img_worker_t));
    if (!sched->workers)
        return -1;

    if (img_mpmc_init(&sched->global_queue, 1024) != 0)
    {
        free(sched->workers);
        sched->workers = NULL;
        return -1;
    }

    /*
     * Workers initialized with sched but ctx=NULL here.
     * ctx MUST be set before pthread_create.
     * The caller (img_api_init_boot_workers) sets ctx on each
     * worker before calling this, or sets it after via the
     * sched->workers[i].ctx field before threads are started.
     *
     * pthread_create is deferred to a separate start call so
     * ctx can be set safely before any thread runs.
     */
    for (uint32_t i = 0; i < workers; i++)
    {
        sched->workers[i].id = i;
        sched->workers[i].scheduler = sched;
        sched->workers[i].ctx = NULL;  /* set by caller before start */
        sched->workers[i].running = 0; /* not running yet */
        sched->workers[i].queue = img_queue_create(10);
        if (!sched->workers[i].queue)
        {
            /* cleanup already created queues */
            for (uint32_t j = 0; j < i; j++)
                img_queue_destroy(sched->workers[j].queue);
            img_mpmc_destroy(&sched->global_queue);
            free(sched->workers);
            sched->workers = NULL;
            return -1;
        }
    }

    return 0;
}

// // ./src/runtime/scheduler_init.c
// #include "runtime/scheduler.h"
// #include "runtime/worker.h"

// #include <stdlib.h>

// int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)
// {
//     if (!sched || workers == 0)
//         return -1;

//     sched->worker_count = workers;

//     sched->workers = calloc(workers, sizeof(img_worker_t));
//     if (!sched->workers)
//         return -1;

//     img_mpmc_init(&sched->global_queue, 1024);

//     for (uint32_t i = 0; i < workers; i++)
//     {
//         /*
//          * Pass sched so workers can steal across queues.
//          * ctx is NULL here — scheduler callers must set
//          * sched->workers[i].ctx after init if needed,
//          * OR pass a pre-built ctx array into a higher-level init.
//          *
//          * img_worker_init sets scheduler BEFORE pthread_create
//          * so the thread never sees a NULL scheduler.
//          */
//         img_worker_init(&sched->workers[i], i, sched, NULL);
//     }

//     return 0;
// }
