// ./src/memory/slab.c
// src/memory/slab.c

#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "memory/numa.h"
#include "memory/poison.h"

#include <stdlib.h>
#include <stdint.h>

static inline size_t align64(size_t x)
{
    return (x + 63) & ~63;
}

img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size)
{
    img_slab_pool_t *pool = malloc(sizeof(img_slab_pool_t));
    if (!pool)
        return NULL;

    block_size = align64(block_size);
    total_size = align64(total_size);

    int node = img_numa_get_node();

    pool->memory = img_numa_alloc_onnode(total_size, node);
    if (!pool->memory)
    {
        free(pool);
        return NULL;
    }

    pool->numa_node = node;
    pool->total_size = total_size;
    pool->block_size = block_size;
    pool->block_count = total_size / block_size;
    pool->free_list = NULL;

    uint8_t *ptr = (uint8_t *)pool->memory;

    for (size_t i = 0; i < pool->block_count; i++)
    {
        slab_block_t *block = (slab_block_t *)(ptr + i * block_size);
        block->next = pool->free_list;
        pool->free_list = block;
    }

    return pool;
}

uint8_t *img_slab_alloc(img_slab_pool_t *pool)
{
    if (!pool || !pool->free_list)
        return NULL;

    slab_block_t *block = pool->free_list;
    pool->free_list = block->next;

    img_poison_block(block, pool->block_size); /* poison on alloc: detect stale reads */

    return (uint8_t *)block;
}

void img_slab_free(img_slab_pool_t *pool, void *ptr)
{
    if (!pool || !ptr)
        return;

    img_poison_block(ptr, pool->block_size); /* poison on free: detect use-after-free */

    slab_block_t *block = (slab_block_t *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
}

void img_slab_destroy(img_slab_pool_t *pool)
{
    if (!pool)
        return;

    img_numa_free(pool->memory, pool->total_size);
    free(pool);
}

size_t img_slab_block_size(img_slab_pool_t *pool)
{
    if (!pool)
        return 0;
    return pool->block_size;
}
