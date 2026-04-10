// ./src/core/memory_pool.c

#include "imgengine/memory_pool.h"
#include <stdlib.h>

#define ALIGN 8

static size_t align_up(size_t x)
{
    return (x + (ALIGN - 1)) & ~(ALIGN - 1);
}

void mp_init(mem_pool_t *mp, size_t size)
{
    mp->buffer = (unsigned char *)malloc(size);
    mp->size = size;
    mp->offset = 0;
}

void *mp_alloc(mem_pool_t *mp, size_t size)
{
    size = align_up(size);

    if (mp->offset + size > mp->size)
        return NULL;

    void *ptr = mp->buffer + mp->offset;
    mp->offset += size;

    return ptr;
}

void mp_reset(mem_pool_t *mp)
{
    mp->offset = 0;
}

void mp_destroy(mem_pool_t *mp)
{
    free(mp->buffer);
}