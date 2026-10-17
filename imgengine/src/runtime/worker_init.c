// ./src/runtime/worker_init.c
#include "runtime/worker.h"
#include "runtime/worker_internal.h"
#include "runtime/affinity.h"

#include <pthread.h>

int img_worker_init(img_worker_t *w, uint32_t id, struct img_scheduler *scheduler,
                    struct img_ctx *ctx) {
    if (!w)
        return -1;

    w->id = id;
    atomic_store_explicit(&w->running, true, memory_order_release);
    w->scheduler = scheduler; /* set BEFORE pthread_create */
    w->ctx = ctx;             /* set BEFORE pthread_create */

    if (!w->queue)
        w->queue = img_queue_create(10);
    if (!w->queue)
        return -1;

    img_pin_thread_to_core(id);

    if (pthread_create(&w->thread, NULL, img_worker_loop, w) != 0) {
        img_queue_destroy(w->queue);
        w->queue = NULL;
        atomic_store_explicit(&w->running, false, memory_order_release);
        return -1;
    }

    w->thread_started = 1;

    return 0;
}
