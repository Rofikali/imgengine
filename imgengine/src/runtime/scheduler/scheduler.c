#include "runtime/scheduler/scheduler.h"
#include "runtime/worker/worker.h"
#include "runtime/queue/mpmc.h"
#include "runtime/queue/spsc.h"

#include <stdlib.h>

int img_scheduler_init(img_scheduler_t *s, uint32_t workers)
{
    s->worker_count = workers;

    s->workers = calloc(workers, sizeof(img_worker_t));
    if (!s->workers) return -1;

    img_mpmc_init(&s->global_queue, 1024);

    for (uint32_t i = 0; i < workers; i++)
    {
        img_worker_init(&s->workers[i], i);
        s->workers[i].scheduler = s;
    }

    return 0;
}

void img_scheduler_destroy(img_scheduler_t *s)
{
    for (uint32_t i = 0; i < s->worker_count; i++)
    {
        img_worker_stop(&s->workers[i]);
        img_worker_join(&s->workers[i]);
    }

    img_mpmc_destroy(&s->global_queue);
    free(s->workers);
}

int img_scheduler_submit(img_scheduler_t *s, img_task_t *task)
{
    uint32_t target = (uintptr_t)task % s->worker_count;

    img_worker_t *w = &s->workers[target];

    if (img_spsc_push(w->queue, task))
        return 0;

    return img_mpmc_push(&s->global_queue, task);
}

img_task_t *img_scheduler_steal(img_scheduler_t *s, uint32_t self)
{
    uint32_t n = s->worker_count;

    for (uint32_t i = 0; i < n; i++)
    {
        uint32_t victim = (self + i + 1) % n;

        img_task_t *t =
            (img_task_t *)img_spsc_pop(s->workers[victim].queue);

        if (t) return t;
    }

    return (img_task_t *)img_mpmc_pop(&s->global_queue);
}