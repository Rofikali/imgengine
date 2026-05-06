// ./src/runtime/queue_create.c
#include "runtime/queue_internal.h"
#include "runtime/queue_spsc.h"

#include <stdlib.h>

img_queue_t *img_queue_create(uint32_t power) {
    img_queue_t *q = malloc(sizeof(img_queue_t));
    if (!q)
        return NULL;

    q->capacity = 1u << power;
    q->mask = q->capacity - 1;

    q->buffer = calloc(q->capacity, sizeof(void *));
    q->head = 0;
    q->tail = 0;

    return q;
}
