/* pipeline/scheduler.c */

// pipeline/scheduler.c

#include "pipeline/scheduler.h"
#include "runtime/worker.h"

// 🔥 Opaque types (kernel-style)
typedef struct img_engine img_engine_t;
typedef struct img_batch img_batch_t;
typedef struct img_pipeline img_pipeline_t;

int img_scheduler_dispatch_batch(
    img_engine_t *engine,
    img_batch_t *batch,
    img_pipeline_desc_t *pipe)
{
    uint32_t idx = __atomic_fetch_add(
        &engine->worker_count,
        1,
        __ATOMIC_RELAXED);

    img_worker_t *w =
        &engine->workers[idx % engine->worker_count];

    // 🔥 PUSH BATCH (lock-free)
    if (!img_queue_push(w->queue, batch))
        return -1;

    return 0;
}
