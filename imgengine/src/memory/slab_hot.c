// ./src/memory/slab_hot.c
#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "memory/poison.h"

uint8_t *img_slab_alloc(img_slab_pool_t *pool) {
    if (!pool)
        return NULL;

    pthread_mutex_lock(&pool->lock);
    if (!pool->free_list) {
        pthread_mutex_unlock(&pool->lock);
        return NULL;
    }

    slab_block_t *block = pool->free_list;
    img_unpoison_block(block, pool->block_size);
    pool->free_list = block->next;
    const size_t index = ((uint8_t *)block - pool->memory) / pool->block_size;
    pool->allocated[index] = 1;
    pthread_mutex_unlock(&pool->lock);

    return (uint8_t *)block;
}

void img_slab_free(img_slab_pool_t *pool, void *ptr) {
    if (!pool || !ptr)
        return;

    const uintptr_t start = (uintptr_t)pool->memory;
    const uintptr_t value = (uintptr_t)ptr;
    if (value < start || value - start >= pool->total_size ||
        (value - start) % pool->block_size != 0)
        return;

    const size_t index = (value - start) / pool->block_size;
    pthread_mutex_lock(&pool->lock);
    if (!pool->allocated[index]) {
        pthread_mutex_unlock(&pool->lock);
        return;
    }

    slab_block_t *block = (slab_block_t *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
    pool->allocated[index] = 0;
    img_poison_block(ptr, pool->block_size);
    pthread_mutex_unlock(&pool->lock);
}

void img_slab_recycle(img_slab_pool_t *pool, void *ptr) { img_slab_free(pool, ptr); }
