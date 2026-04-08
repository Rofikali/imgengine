#define _GNU_SOURCE
#include "memory/slab/slab.h"
#include "memory/slab/slab_internal.h"

#include <stdlib.h>
#include <string.h>

static inline size_t align64(size_t x)
{
    return (x + 63) & ~63ULL;
}

img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size)
{
    if (total_size == 0 || block_size == 0)
        return NULL;

    block_size = align64(block_size);

    img_slab_pool_t *pool = aligned_alloc(64, sizeof(*pool));
    if (!pool)
        return NULL;

    memset(pool, 0, sizeof(*pool));

    pool->base = aligned_alloc(64, total_size);
    if (!pool->base)
        return NULL;

    pool->block_size = block_size;
    pool->total_blocks = total_size / block_size;

    slab_block_t *prev = NULL;

    for (size_t i = 0; i < pool->total_blocks; i++)
    {
        slab_block_t *b = (slab_block_t *)(pool->base + i * block_size);
        b->next = prev;
        prev = b;
    }

    atomic_store(&pool->free_list, prev);

    return pool;
}

void *img_slab_alloc(img_slab_pool_t *pool)
{
    if (!pool)
        return NULL;

    slab_block_t *head;

    do
    {
        head = atomic_load(&pool->free_list);
        if (!head)
            return NULL;

    } while (!atomic_compare_exchange_weak(
        &pool->free_list,
        &head,
        head->next));

    return (void *)head;
}

void img_slab_free(img_slab_pool_t *pool, void *ptr)
{
    if (!pool || !ptr)
        return;

    slab_block_t *block = (slab_block_t *)ptr;
    slab_block_t *head;

    do
    {
        head = atomic_load(&pool->free_list);
        block->next = head;

    } while (!atomic_compare_exchange_weak(
        &pool->free_list,
        &head,
        block));
}

void img_slab_destroy(img_slab_pool_t *pool)
{
    if (!pool)
        return;

    free(pool->base);
    free(pool);
}

size_t img_slab_block_size(img_slab_pool_t *pool)
{
    return pool ? pool->block_size : 0;
}