// ./src/runtime/queue_mpmc_push.c
#include "runtime/queue_mpmc.h"

int img_mpmc_push(img_mpmc_queue_t *q, void *data) {
    cell_t *cell;
    size_t pos;

    for (;;) {
        pos = atomic_load_explicit(&q->tail, memory_order_relaxed);
        cell = &q->cells[pos & q->mask];

        size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;

        if (diff == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->tail, &pos, pos + 1, memory_order_relaxed,
                                                      memory_order_relaxed))
                break;
        } else if (diff < 0) {
            return -1;
        } else {
            __builtin_ia32_pause();
        }
    }

    cell->data = data;
    atomic_store_explicit(&cell->seq, pos + 1, memory_order_release);
    return 0;
}
