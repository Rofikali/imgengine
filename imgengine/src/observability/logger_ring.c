// ./src/observability/logger_ring.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"

#include <stdatomic.h>
#include <stdint.h>
#include <string.h>

log_ring_t g_ring;

int log_ring_push(log_ring_t *q, const char *msg) {
    log_cell_t *cell;
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
        }
    }

    memcpy(cell->msg, msg, LOG_MSG_SIZE);

    atomic_store_explicit(&cell->seq, pos + 1, memory_order_release);

    return 0;
}

int log_ring_pop(log_ring_t *q, char *out) {
    log_cell_t *cell;
    size_t pos;

    pos = atomic_load_explicit(&q->head, memory_order_relaxed);
    cell = &q->cells[pos & q->mask];

    size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
    intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

    if (diff < 0)
        return -1;

    if (!atomic_compare_exchange_weak_explicit(&q->head, &pos, pos + 1, memory_order_relaxed,
                                               memory_order_relaxed))
        return -1;

    memcpy(out, cell->msg, LOG_MSG_SIZE);

    atomic_store_explicit(&cell->seq, pos + q->mask + 1, memory_order_release);

    return 0;
}
