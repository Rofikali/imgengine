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

// #include "runtime/queue_mpmc.h"

// #ifndef IMGENGINE_RUNTIME_WORKER_H
// #define IMGENGINE_RUNTIME_WORKER_H

// #include <pthread.h>
// #include <stdbool.h>
// #include "core/context.h"
// #include "runtime/queue_spsc.h"

// typedef struct
// {
//     pthread_t thread;
//     uint32_t cpu_id;
//     img_ctx_t *ctx;
//     // img_queue_t *task_queue;
//     volatile bool active;
//     // int cpu_id;
//     // int active;

//     img_mpmc_queue_t *task_queue; // 🔥 CHANGE THIS

// } img_worker_t;

// /**
//  * Worker main loop
//  */
// void *img_worker_loop(void *arg);

// #endif