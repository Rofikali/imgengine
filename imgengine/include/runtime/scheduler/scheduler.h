#ifndef IMGENGINE_SCHEDULER_H
#define IMGENGINE_SCHEDULER_H

#include <stdint.h>

typedef struct img_worker img_worker_t;
typedef struct img_task img_task_t;
typedef struct img_mpmc_queue img_mpmc_queue_t;

typedef struct img_scheduler
{
    uint32_t worker_count;

    img_worker_t *workers;

    img_mpmc_queue_t global_queue;

} img_scheduler_t;

int img_scheduler_init(img_scheduler_t *s, uint32_t workers);
void img_scheduler_destroy(img_scheduler_t *s);

int img_scheduler_submit(img_scheduler_t *s, img_task_t *task);
img_task_t *img_scheduler_steal(img_scheduler_t *s, uint32_t self);

#endif