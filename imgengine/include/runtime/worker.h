/* include/runtime/worker.h */

#ifndef IMGENGINE_RUNTIME_WORKER_H
#define IMGENGINE_RUNTIME_WORKER_H

#include <pthread.h>
#include <stdint.h>

#include "runtime/queue_spsc.h"
#include "runtime/task.h"

/*
 * Forward declarations (NO heavy includes)
 */
typedef struct img_slab_pool img_slab_pool_t;
typedef struct img_arena img_arena_t;
typedef struct img_ctx img_ctx_t;

/*
 * 🔥 Worker structure (CACHE FRIENDLY)
 */
typedef struct img_worker_s
{
    uint32_t id;

    pthread_t thread;

    // 🔥 LOCK-FREE QUEUE
    img_queue_t *queue;

    // 🔥 MEMORY LOCALITY
    img_slab_pool_t *slab;
    img_arena_t *arena;

    // 🔥 EXECUTION CONTEXT
    img_ctx_t *ctx;

    // 🔥 CONTROL
    volatile int running;

} img_worker_t;

/*
 * Lifecycle
 */
int img_worker_init(img_worker_t *w, uint32_t id);
void img_worker_stop(img_worker_t *w);
void img_worker_join(img_worker_t *w);

#endif