/* runtime/worker.h */

#ifndef IMGENGINE_RUNTIME_WORKER_H
#define IMGENGINE_RUNTIME_WORKER_H

#include <pthread.h>
#include "runtime/queue_spsc.h"
#include "memory/slab.h"
#include "memory/arena.h"
#include "core/context_internal.h"
#include "runtime/task.h"
#include "api/v1/img_buffer_utils.h"

typedef struct img_worker_s
{
    uint32_t id;

    pthread_t thread;

    // 🔥 Lock-free queue
    img_queue_t *queue;

    // 🔥 Memory locality
    img_slab_pool_t *slab;
    img_arena_t *arena;

    // 🔥 Execution context
    img_ctx_t ctx;

    // 🔥 Control
    volatile int running;

} img_worker_t;

#endif
