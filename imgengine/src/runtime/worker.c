/* src/runtime/worker.c */

#include "runtime/worker.h"
#include "runtime/affinity.h"

#include "pipeline/engine.h"
#include "pipeline/batch_exec.h"
#include "observability/metrics.h"
#include "observability/profiler.h"

#include "memory/slab.h"
#include "memory/arena.h"
#include "core/context_internal.h"

#include <stdlib.h>

void *worker_loop(void *arg)
{
    img_worker_t *w = (img_worker_t *)arg;

    while (__atomic_load_n(&w->running, __ATOMIC_RELAXED))
    {
        void *item = img_queue_pop(w->queue);

        if (!item)
            continue;

        // 🔥 BATCH EXECUTION
        img_batch_t *batch = (img_batch_t *)item;

        img_batch_execute(
            &w->ctx,
            batch,
            NULL);
    }

    return NULL;
}

int img_worker_init(img_worker_t *w, uint32_t id)
{
    if (!w)
        return -1;

    w->id = id;
    w->running = 1;

    w->slab = img_slab_create(64 * 1024 * 1024, 256 * 1024);
    w->arena = img_arena_create(1024 * 1024);

    if (!w->slab || !w->arena)
        return -1;

    w->queue = img_queue_create(10);

    // ✅ INLINE CONTEXT INIT (CORRECT)
    w->ctx.thread_id = id;
    w->ctx.local_pool = w->slab;
    w->ctx.scratch_arena = w->arena;
    w->ctx.caps = img_cpu_detect_caps();

    pthread_create(&w->thread, NULL, worker_loop, w);

    return 0;
}
