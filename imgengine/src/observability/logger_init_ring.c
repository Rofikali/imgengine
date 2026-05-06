// ./src/observability/logger_init_ring.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"
#include "core/align.h"

#include <stdlib.h>

int img_logger_init_ring(void) {
    size_t size = img_align_pow2(LOG_RING_SIZE);

    g_ring.size = size;
    g_ring.mask = size - 1;

    g_ring.cells = aligned_alloc(64, sizeof(log_cell_t) * size);
    if (!g_ring.cells)
        return -1;

    for (size_t i = 0; i < size; i++)
        atomic_init(&g_ring.cells[i].seq, i);

    atomic_init(&g_ring.head, 0);
    atomic_init(&g_ring.tail, 0);
    return 0;
}
