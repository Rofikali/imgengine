// ./src/runtime/worker_control.c
#include "runtime/worker.h"

void img_worker_stop(img_worker_t *w) {
    if (w)
        atomic_store_explicit(&w->running, false, memory_order_release);
}

void img_worker_join(img_worker_t *w) {
    if (w && w->thread_started) {
        pthread_join(w->thread, NULL);
        w->thread_started = 0;
    }
}
