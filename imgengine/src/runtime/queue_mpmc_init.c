// ./src/runtime/queue_mpmc_init.c
#include "runtime/queue_mpmc.h"
#include "core/align.h"

#include <stdlib.h>
#include <string.h>

int img_mpmc_init(img_mpmc_queue_t *q, size_t size) {
    if (!q || size == 0)
        return -1;

    size = img_align_pow2(size);

    q->size = size;
    q->mask = size - 1;

    q->cells = aligned_alloc(64, sizeof(cell_t) * size);
    if (!q->cells)
        return -1;

    memset(q->cells, 0, sizeof(cell_t) * size);

    for (size_t i = 0; i < size; i++) {
        atomic_init(&q->cells[i].seq, i);
        q->cells[i].data = NULL;
    }

    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    return 0;
}
