// /* src/runtime/worker.c */

#include "runtime/worker.h"
#include "runtime/task.h"
#include "runtime/scheduler.h"
#include "hot/pipeline_exec.h"
#include "runtime/affinity.h"

#include <stdlib.h>
#include <immintrin.h>

#include "observability/binlog.h"

extern void img_binlog_set_cpu(uint32_t cpu);

static void *worker_loop(void *arg)
{
    img_worker_t *w = (img_worker_t *)arg;

    /*
     * 🔥 CRITICAL: bind CPU ID to TLS
     */
    img_binlog_set_cpu(w->id);

    while (__builtin_expect(w->running, 1))
    {
        img_task_t *task =
            (img_task_t *)img_queue_pop(w->queue);

        if (!task)
        {
            task = img_scheduler_steal(
                w->scheduler,
                w->id);
        }

        if (!task)
        {
            _mm_pause();
            continue;
        }

        img_pipeline_execute_hot(
            w->ctx,
            task->pipeline,
            task->buffer);

        task->status = 0;
    }

    return NULL;
}

int img_worker_init(img_worker_t *w, uint32_t id)
{
    if (!w)
        return -1;

    w->id = id;
    w->running = 1;

    // 🔥 SPSC queue (FAST PATH)
    w->queue = img_queue_create(1024);

    if (!w->queue)
        return -1;

    img_pin_thread_to_core(id);

    pthread_create(&w->thread, NULL, worker_loop, w);

    return 0;
}

void img_worker_stop(img_worker_t *w)
{
    if (w)
        w->running = 0;
}

void img_worker_join(img_worker_t *w)
{
    if (w)
        pthread_join(w->thread, NULL);
}
