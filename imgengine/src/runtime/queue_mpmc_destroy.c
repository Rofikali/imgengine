// ./src/runtime/queue_mpmc_destroy.c
#include "runtime/queue_mpmc.h"

#include <stdlib.h>

void img_mpmc_destroy(img_mpmc_queue_t *q) {
    if (!q)
        return;

    free(q->cells);
}
