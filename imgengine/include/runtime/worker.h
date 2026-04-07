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

// #ifndef IMGENGINE_RUNTIME_WORKER_H
// #define IMGENGINE_RUNTIME_WORKER_H

// #include <stdint.h>
// #include "runtime/queue_spsc.h"

// struct img_scheduler;

// typedef struct img_worker
// {
//     uint32_t id;

//     struct img_scheduler *scheduler;

//     // 🔥 PER-WORKER SPSC QUEUE (FAST PATH)
//     img_queue_t *queue;

// } img_worker_t;

// void img_worker_init(img_worker_t *w, uint32_t id);
// void img_worker_stop(img_worker_t *w);
// void img_worker_join(img_worker_t *w);

// #endif

// // #ifndef IMGENGINE_RUNTIME_WORKER_H
// // #define IMGENGINE_RUNTIME_WORKER_H

// // #include <pthread.h>
// // #include <stdint.h>

// // #include "runtime/queue_spsc.h"
// // #include "runtime/queue_mpmc.h"
// // #include "runtime/task.h"

// // /*
// //  * Forward declarations (NO heavy includes)
// //  */
// // typedef struct img_slab_pool img_slab_pool_t;
// // typedef struct img_arena img_arena_t;
// // typedef struct img_ctx img_ctx_t;
// // typedef struct img_scheduler img_scheduler_t;

// // /*
// //  * 🔥 Worker structure (CACHE FRIENDLY)
// //  */

// // #include "runtime/queue_mpmc.h"

// // typedef struct img_worker_s
// // {
// //     uint32_t id;

// //     pthread_t thread;

// //     // 🔥 USE MPMC (NOT SPSC)
// //     img_mpmc_queue_t *queue;

// //     img_slab_pool_t *slab;
// //     img_arena_t *arena;

// //     img_ctx_t *ctx;

// //     img_scheduler_t *scheduler; // 🔥 ADD THIS

// //     volatile int running;

// // } img_worker_t;

// // /*
// //  * Lifecycle
// //  */
// // int img_worker_init(img_worker_t *w, uint32_t id);
// // void img_worker_stop(img_worker_t *w);
// // void img_worker_join(img_worker_t *w);

// // #endif
