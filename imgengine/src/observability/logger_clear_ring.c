// ./src/observability/logger_clear_ring.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"

#include <stdlib.h>

void img_logger_clear_ring(void) {
    if (!g_ring.cells)
        return;

    free(g_ring.cells);
    g_ring.cells = NULL;
    g_ring.size = 0;
    g_ring.mask = 0;
}
