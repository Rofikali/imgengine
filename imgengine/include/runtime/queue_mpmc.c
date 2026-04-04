#include "runtime/queue_mpmc.h"
#include <stdlib.h>

typedef struct cell_t
{
    _Atomic size_t seq;
    void *data;
} cell_t;

int img_mpmc_init(img_mpmc_queue_t *q, size_t size)
{
    // must be power of 2
    if ((size & (size - 1)) != 0)
        return -1;

    q->size = size;
    q->mask = size - 1;

    q->cells = aligned_alloc(64, sizeof(cell_t) * size);
    if (!q->cells)
        return -1;

    for (size_t i = 0; i < size; i++)
    {
        atomic_store(&q->cells[i].seq, i);
    }

    atomic_store(&q->head, 0);
    atomic_store(&q->tail, 0);

    return 0;
}

int img_mpmc_push(img_mpmc_queue_t *q, void *data)
{
    cell_t *cell;
    size_t pos = atomic_load(&q->tail);

    for (;;)
    {
        cell = &q->cells[pos & q->mask];
        size_t seq = atomic_load(&cell->seq);

        intptr_t diff = (intptr_t)seq - (intptr_t)pos;

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&q->tail, &pos, pos + 1))
                break;
        }
        else if (diff < 0)
        {
            return -1; // queue full
        }
        else
        {
            pos = atomic_load(&q->tail);
        }
    }

    cell->data = data;
    atomic_store(&cell->seq, pos + 1);

    return 0;
}
void *img_mpmc_pop(img_mpmc_queue_t *q)
{
    cell_t *cell;
    size_t pos = atomic_load(&q->head);

    for (;;)
    {
        cell = &q->cells[pos & q->mask];
        size_t seq = atomic_load(&cell->seq);

        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&q->head, &pos, pos + 1))
                break;
        }
        else if (diff < 0)
        {
            return NULL; // empty
        }
        else
        {
            pos = atomic_load(&q->head);
        }
    }

    void *data = cell->data;
    atomic_store(&cell->seq, pos + q->mask + 1);

    return data;
}