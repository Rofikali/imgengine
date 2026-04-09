#ifndef IMGENGINE_WORKER_H
#define IMGENGINE_WORKER_H

#include <pthread.h>
#include <stdint.h>

typedef struct img_scheduler img_scheduler_t;
typedef struct img_spsc_queue img_spsc_queue_t;
typedef struct img_ctx img_ctx_t;

typedef struct img_worker
{
    uint32_t id;
    int running;

    pthread_t thread;

    img_ctx_t *ctx;

    img_spsc_queue_t *queue;

    img_scheduler_t *scheduler;

} img_worker_t;

int img_worker_init(img_worker_t *w, uint32_t id);
void img_worker_stop(img_worker_t *w);
void img_worker_join(img_worker_t *w);

#endif