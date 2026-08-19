// ./src/memory/slab_create.c
#include "memory/align.h"
#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "memory/numa.h"

#include <stdint.h>
#include <stdlib.h>

img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size) {
    if (total_size == 0 || block_size == 0 || total_size > SIZE_MAX - 63u ||
        block_size > SIZE_MAX - 63u)
        return NULL;

    img_slab_pool_t *pool = malloc(sizeof(img_slab_pool_t));
    if (!pool)
        return NULL;

    block_size = img_align64(block_size);
    total_size = img_align64(total_size);
    if (total_size < block_size) {
        free(pool);
        return NULL;
    }

    int node = img_numa_get_node();

    pool->memory = img_numa_alloc_onnode(total_size, node);
    if (!pool->memory) {
        free(pool);
        return NULL;
    }

    pool->numa_node = node;
    pool->total_size = total_size;
    pool->block_size = block_size;
    pool->block_count = total_size / block_size;
    pool->free_list = NULL;

    uint8_t *ptr = (uint8_t *)pool->memory;

    for (size_t i = 0; i < pool->block_count; i++) {
        slab_block_t *block = (slab_block_t *)(ptr + i * block_size);
        block->next = pool->free_list;
        pool->free_list = block;
    }

    return pool;
}
