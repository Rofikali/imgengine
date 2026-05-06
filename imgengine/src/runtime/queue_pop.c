// ./src/runtime/queue_pop.c
#include "runtime/queue_internal.h"
#include "runtime/queue_spsc.h"

#include <stddef.h>

void *img_queue_pop(img_queue_t *q) {
    uint32_t h = q->head;

    if (h == __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE))
        return NULL;

    void *data = q->buffer[h];
    __atomic_store_n(&q->head, (h + 1) & q->mask, __ATOMIC_RELEASE);

    return data;
}
