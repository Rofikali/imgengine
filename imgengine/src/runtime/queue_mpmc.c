// // include/runtime/queue_mpmc.c

#include "runtime/queue_mpmc.h"
#include <stdlib.h>
#include <string.h>

/*
 * 🔥 round to power of 2
 */
static inline size_t align_pow2(size_t x)
{
    size_t p = 1;
    while (p < x)
        p <<= 1;
    return p;
}

int img_mpmc_init(img_mpmc_queue_t *q, size_t size)
{
    if (!q || size == 0)
        return -1;

    size = align_pow2(size);

    q->size = size;
    q->mask = size - 1;

    q->cells = aligned_alloc(64, sizeof(cell_t) * size);
    if (!q->cells)
        return -1;

    memset(q->cells, 0, sizeof(cell_t) * size);

    for (size_t i = 0; i < size; i++)
    {
        atomic_init(&q->cells[i].seq, i);
        q->cells[i].data = NULL;
    }

    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);

    return 0;
}

void img_mpmc_destroy(img_mpmc_queue_t *q)
{
    if (!q)
        return;

    free(q->cells);
}

/*
 * 🔥 MULTI-PRODUCER PUSH
 */
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
            if (atomic_compare_exchange_weak_explicit(
                    &q->tail,
                    &pos,
                    pos + 1,
                    memory_order_relaxed,
                    memory_order_relaxed))
                break;
        }
        else if (diff < 0)
        {
            return -1; // FULL
        }
        else
        {
            __builtin_ia32_pause();
        }
    }

    cell->data = data;

    atomic_store_explicit(
        &cell->seq,
        pos + 1,
        memory_order_release);

    return 0;
}

/*
 * 🔥 MULTI-CONSUMER POP
 */
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
            if (atomic_compare_exchange_weak_explicit(
                    &q->head,
                    &pos,
                    pos + 1,
                    memory_order_relaxed,
                    memory_order_relaxed))
                break;
        }
        else if (diff < 0)
        {
            return NULL; // EMPTY
        }
        else
        {
            __builtin_ia32_pause();
        }
    }

    void *data = cell->data;

    atomic_store_explicit(
        &cell->seq,
        pos + q->mask + 1,
        memory_order_release);

    return data;
}
