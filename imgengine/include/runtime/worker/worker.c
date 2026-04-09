#include "runtime/worker/worker.h"
#include "runtime/scheduler/scheduler.h"
#include "runtime/queue/spsc.h"
#include "pipeline/exec/pipeline_exec.h"
#include "runtime/affinity/affinity.h"

#include <immintrin.h>
#include <stdlib.h>

typedef struct img_task img_task_t;

static void *worker_loop(void *arg)
{
    img_worker_t *w = arg;

    img_pin_thread_to_core(w->id);

    while (__builtin_expect(w->running, 1))
    {
        img_task_t *task =
            (img_task_t *)img_spsc_pop(w->queue);

        if (!task)
        {
            task = img_scheduler_steal(w->scheduler, w->id);
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
    w->id = id;
    w->running = 1;

    w->queue = img_spsc_create(10); // 1024
    if (!w->queue) return -1;

    pthread_create(&w->thread, NULL, worker_loop, w);

    return 0;
}

void img_worker_stop(img_worker_t *w)
{
    w->running = 0;
}

void img_worker_join(img_worker_t *w)
{
    pthread_join(w->thread, NULL);
}