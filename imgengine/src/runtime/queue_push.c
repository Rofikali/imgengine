// ./src/runtime/queue_push.c
#include "runtime/queue_internal.h"
#include "runtime/queue_spsc.h"

bool img_queue_push(img_queue_t *q, void *data) {
    uint32_t t = q->tail;
    uint32_t next = (t + 1) & q->mask;

    if (next == __atomic_load_n(&q->head, __ATOMIC_ACQUIRE))
        return false;

    q->buffer[t] = data;
    __atomic_store_n(&q->tail, next, __ATOMIC_RELEASE);

    return true;
}
