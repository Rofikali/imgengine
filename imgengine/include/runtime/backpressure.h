// ./include/runtime/backpressure.h

/* runtime/backpressure.h */

#ifndef IMGENGINE_RUNTIME_BACKPRESSURE_H
#define IMGENGINE_RUNTIME_BACKPRESSURE_H

#include <stdbool.h>
#include <stdint.h>

#include "runtime/queue_spsc.h"
#include "runtime/queue_mpmc.h"

/*
 * 🔥 SPSC BACKPRESSURE (worker-local queues)
 */
static inline bool img_backpressure_check_spsc(img_queue_t *q) {
    if (!q)
        return true;

    uint32_t h = __atomic_load_n(&q->head, __ATOMIC_ACQUIRE);
    uint32_t t = __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE);

    uint32_t usage = (t - h) & q->mask;

    return usage > (uint32_t)(q->capacity * 0.9);
}

/*
 * 🔥 MPMC BACKPRESSURE (global queue)
 */
static inline bool img_backpressure_check_mpmc(img_mpmc_queue_t *q) {
    if (!q)
        return true;

    size_t head = atomic_load_explicit(&q->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);

    return (tail - head) > (q->size * 3 / 4);
}

#endif
