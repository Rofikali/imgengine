/* runtime/backpressure.c */
#include "runtime/backpressure.h"

struct img_queue
{
    void **buffer;
    uint32_t capacity;
    uint32_t mask;
    uint32_t head;
    uint32_t tail;
};

bool img_backpressure_check(img_queue_t *q)
{
    if (!q)
        return true;

    uint32_t h = __atomic_load_n(&q->head, __ATOMIC_ACQUIRE);
    uint32_t t = __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE);

    uint32_t usage = (t - h) & q->mask;

    return usage > (uint32_t)(q->capacity * 0.9);
}

