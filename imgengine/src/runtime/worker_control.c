// ./src/runtime/worker_control.c
#include "runtime/worker.h"

void img_worker_stop(img_worker_t *w) {
    if (w)
        w->running = 0;
}

void img_worker_join(img_worker_t *w) {
    if (w)
        pthread_join(w->thread, NULL);
}
