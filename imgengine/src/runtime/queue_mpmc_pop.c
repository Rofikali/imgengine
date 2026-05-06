// ./src/runtime/queue_mpmc_pop.c
#include "runtime/queue_mpmc.h"

void *img_mpmc_pop(img_mpmc_queue_t *q) {
    cell_t *cell;
    size_t pos;

    for (;;) {
        pos = atomic_load_explicit(&q->head, memory_order_relaxed);
        cell = &q->cells[pos & q->mask];

        size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

        if (diff == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->head, &pos, pos + 1, memory_order_relaxed,
                                                      memory_order_relaxed))
                break;
        } else if (diff < 0) {
            return NULL;
        } else {
            __builtin_ia32_pause();
        }
    }

    void *data = cell->data;
    atomic_store_explicit(&cell->seq, pos + q->mask + 1, memory_order_release);
    return data;
}
