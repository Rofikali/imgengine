// ./src/runtime/queue_mpmc_init.c
#include "runtime/queue_mpmc.h"
#include "core/align.h"

#include <stdlib.h>
#include <string.h>

int img_mpmc_init(img_mpmc_queue_t *q, size_t size) {
    if (!q || size == 0)
        return -1;

    size_t capacity = 1;
    while (capacity < size) {
        if (capacity > SIZE_MAX / 2u)
            return -1;
        capacity <<= 1u;
    }
    if (capacity > SIZE_MAX / sizeof(cell_t))
        return -1;

    const size_t cell_bytes = capacity * sizeof(cell_t);
    if (cell_bytes > SIZE_MAX - 63u)
        return -1;
    const size_t allocation_bytes = (cell_bytes + 63u) & ~(size_t)63u;

    cell_t *cells = aligned_alloc(64, allocation_bytes);
    if (!cells)
        return -1;

    memset(cells, 0, cell_bytes);

    for (size_t i = 0; i < capacity; i++) {
        atomic_init(&cells[i].seq, i);
        cells[i].data = NULL;
    }

    q->size = capacity;
    q->mask = capacity - 1u;
    q->cells = cells;
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    return 0;
}
