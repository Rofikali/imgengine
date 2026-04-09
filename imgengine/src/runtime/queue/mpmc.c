#include "runtime/queue/mpmc.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

typedef struct
{
    _Atomic size_t seq;
    void *data;
} cell_t;

struct img_mpmc_queue
{
    size_t size;
    size_t mask;

    cell_t *cells;

    alignas(64) _Atomic size_t head;
    alignas(64) _Atomic size_t tail;
};

static size_t pow2(size_t x)
{
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

int img_mpmc_init(img_mpmc_queue_t *q, size_t size)
{
    size = pow2(size);

    q->size = size;
    q->mask = size - 1;

    q->cells = aligned_alloc(64, sizeof(cell_t) * size);
    if (!q->cells) return -1;

    for (size_t i = 0; i < size; i++)
        atomic_init(&q->cells[i].seq, i);

    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);

    return 0;
}

void img_mpmc_destroy(img_mpmc_queue_t *q)
{
    free(q->cells);
}

int img_mpmc_push(img_mpmc_queue_t *q, void *data)
{
    cell_t *cell;
    size_t pos;

    for (;;)
    {
        pos = atomic_load_explicit(&q->tail, memory_order_relaxed);
        cell = &q->cells[pos & q->mask];

        size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&q->tail, &pos, pos + 1))
                break;
        }
        else if (diff < 0)
            return -1;
    }

    cell->data = data;
    atomic_store_explicit(&cell->seq, pos + 1, memory_order_release);

    return 0;
}

void *img_mpmc_pop(img_mpmc_queue_t *q)
{
    cell_t *cell;
    size_t pos;

    for (;;)
    {
        pos = atomic_load_explicit(&q->head, memory_order_relaxed);
        cell = &q->cells[pos & q->mask];

        size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&q->head, &pos, pos + 1))
                break;
        }
        else if (diff < 0)
            return NULL;
    }

    void *data = cell->data;
    atomic_store_explicit(&cell->seq, pos + q->mask + 1, memory_order_release);

    return data;
}