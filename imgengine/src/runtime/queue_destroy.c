// ./src/runtime/queue_destroy.c
#include "runtime/queue_internal.h"
#include "runtime/queue_spsc.h"

#include <stdlib.h>

void img_queue_destroy(img_queue_t *q) {
    if (!q)
        return;

    free(q->buffer);
    free(q);
}
