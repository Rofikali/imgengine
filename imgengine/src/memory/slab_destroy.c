// ./src/memory/slab_destroy.c
#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "memory/numa.h"

#include <stdlib.h>

void img_slab_destroy(img_slab_pool_t *pool) {
    if (!pool)
        return;

    pthread_mutex_destroy(&pool->lock);
    free(pool->allocated);
    img_numa_free(pool->memory, pool->total_size);
    free(pool);
}
