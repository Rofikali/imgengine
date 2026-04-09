#include "runtime/queue/spsc.h"
#include <stdlib.h>
#include <stdalign.h>

struct img_spsc_queue
{
    void **buffer;
    uint32_t mask;
    uint32_t capacity;

    alignas(64) uint32_t head;
    alignas(64) uint32_t tail;
};

img_spsc_queue_t *img_spsc_create(uint32_t power)
{
    img_spsc_queue_t *q = malloc(sizeof(*q));
    if (!q) return NULL;

    q->capacity = 1u << power;
    q->mask = q->capacity - 1;

    q->buffer = calloc(q->capacity, sizeof(void *));
    if (!q->buffer) return NULL;

    q->head = 0;
    q->tail = 0;

    return q;
}

void img_spsc_destroy(img_spsc_queue_t *q)
{
    if (!q) return;
    free(q->buffer);
    free(q);
}

bool img_spsc_push(img_spsc_queue_t *q, void *data)
{
    uint32_t t = q->tail;
    uint32_t next = (t + 1) & q->mask;

    if (next == __atomic_load_n(&q->head, __ATOMIC_ACQUIRE))
        return false;

    q->buffer[t] = data;
    __atomic_store_n(&q->tail, next, __ATOMIC_RELEASE);

    return true;
}

void *img_spsc_pop(img_spsc_queue_t *q)
{
    uint32_t h = q->head;

    if (h == __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE))
        return NULL;

    void *data = q->buffer[h];
    __atomic_store_n(&q->head, (h + 1) & q->mask, __ATOMIC_RELEASE);

    return data;
}