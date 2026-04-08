// /* include/runtime/worker.h */

#ifndef IMGENGINE_RUNTIME_WORKER_H
#define IMGENGINE_RUNTIME_WORKER_H

#include <stdint.h>
#include <pthread.h>

#include "runtime/queue_spsc.h"

struct img_scheduler;
struct img_ctx;

/*
 * 🔥 FINAL WORKER STRUCT (L9 GRADE)
 */
typedef struct img_worker
{
    uint32_t id;

    pthread_t thread;

    // 🔥 FAST PATH (SPSC)
    img_queue_t *queue;

    struct img_scheduler *scheduler;

    struct img_ctx *ctx;

    volatile int running;

} img_worker_t;

/*
 * Lifecycle
 */
int img_worker_init(img_worker_t *w, uint32_t id);
void img_worker_stop(img_worker_t *w);
void img_worker_join(img_worker_t *w);

#endif
