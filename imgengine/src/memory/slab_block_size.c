// ./src/memory/slab_block_size.c
#include "memory/slab.h"
#include "memory/slab_internal.h"

size_t img_slab_block_size(img_slab_pool_t *pool) {
    if (!pool)
        return 0;
    return pool->block_size;
}
