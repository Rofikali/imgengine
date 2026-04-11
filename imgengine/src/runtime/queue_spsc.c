// ./src/runtime/queue_spsc.c

// ./src/runtime/queue_spsc.c

#include "runtime/queue_spsc.h"
#include <stdlib.h>
#include <stdalign.h>

struct img_queue
{
    void **buffer;
    uint32_t capacity;
    uint32_t mask;

    alignas(64) uint32_t head;
    alignas(64) uint32_t tail;
};

img_queue_t *img_queue_create(uint32_t power)
{
    img_queue_t *q = malloc(sizeof(img_queue_t));
    if (!q)
        return NULL;

    q->capacity = 1 << power;
    q->mask = q->capacity - 1;

    q->buffer = calloc(q->capacity, sizeof(void *));
    q->head = 0;
    q->tail = 0;

    return q;
}

bool img_queue_push(img_queue_t *q, void *data)
{
    uint32_t t = q->tail;
    uint32_t next = (t + 1) & q->mask;

    if (next == __atomic_load_n(&q->head, __ATOMIC_ACQUIRE))
        return false; // full

    q->buffer[t] = data;
    __atomic_store_n(&q->tail, next, __ATOMIC_RELEASE);

    return true;
}

void *img_queue_pop(img_queue_t *q)
{

    // task = img_queue_pop(w->queue);
    // if (!task)
    //     continue;
    uint32_t h = q->head;

    if (h == __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE))
        return NULL; // empty

    void *data = q->buffer[h];
    __atomic_store_n(&q->head, (h + 1) & q->mask, __ATOMIC_RELEASE);

    return data;
}