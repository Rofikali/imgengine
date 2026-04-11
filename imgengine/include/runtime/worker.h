// include/runtime/worker.h

// ================================================================
// FILE: include/runtime/worker.h
// CHANGE: img_worker_init() takes scheduler and ctx pointers.
//         Both must be set before the thread starts.
// ================================================================

#ifndef IMGENGINE_RUNTIME_WORKER_H
#define IMGENGINE_RUNTIME_WORKER_H

#include <stdint.h>
#include <pthread.h>

#include "runtime/queue_spsc.h"

struct img_scheduler;
struct img_ctx;

typedef struct img_worker
{
    uint32_t id;
    pthread_t thread;

    img_queue_t *queue;
    struct img_scheduler *scheduler;
    struct img_ctx *ctx;

    volatile int running;

} img_worker_t;

/*
 * scheduler and ctx MUST be valid before img_worker_init() is called.
 * The worker thread starts immediately — it reads both before any
 * other code can set them after the fact.
 */
int img_worker_init(img_worker_t *w, uint32_t id,
                    struct img_scheduler *scheduler,
                    struct img_ctx *ctx);

void img_worker_stop(img_worker_t *w);
void img_worker_join(img_worker_t *w);

#endif /* IMGENGINE_RUNTIME_WORKER_H */

// // ./include/runtime/worker.h

// #ifndef IMGENGINE_RUNTIME_WORKER_H
// #define IMGENGINE_RUNTIME_WORKER_H

// #include <stdint.h>
// #include <pthread.h>

// #include "runtime/queue_spsc.h"

// struct img_scheduler;
// struct img_ctx;

// /*
//  * 🔥 FINAL WORKER STRUCT (L9 GRADE)
//  */
// typedef struct img_worker
// {
//     uint32_t id;

//     pthread_t thread;

//     // 🔥 FAST PATH (SPSC)
//     img_queue_t *queue;

//     struct img_scheduler *scheduler;

//     struct img_ctx *ctx;

//     volatile int running;

// } img_worker_t;

// /*
//  * Lifecycle
//  */
// int img_worker_init(img_worker_t *w, uint32_t id);
// void img_worker_stop(img_worker_t *w);
// void img_worker_join(img_worker_t *w);

// #endif
