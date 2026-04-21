// ./src/memory/slab_hot.c
#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "memory/poison.h"

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

void img_slab_recycle(img_slab_pool_t *pool, void *ptr)
{
    img_slab_free(pool, ptr);
}
