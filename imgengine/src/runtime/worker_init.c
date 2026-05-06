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
    w->running = 1;
    w->scheduler = scheduler; /* set BEFORE pthread_create */
    w->ctx = ctx;             /* set BEFORE pthread_create */

    w->queue = img_queue_create(10);
    if (!w->queue)
        return -1;

    img_pin_thread_to_core(id);

    if (pthread_create(&w->thread, NULL, img_worker_loop, w) != 0) {
        img_queue_destroy(w->queue);
        w->queue = NULL;
        w->running = 0;
        return -1;
    }

    return 0;
}
