#include "memory/arena/arena.h"
#include <stdlib.h>

static inline size_t align16(size_t x)
{
    return (x + 15) & ~15ULL;
}

int img_arena_init(img_arena_t *a, size_t size)
{
    if (!a || size == 0)
        return -1;

    a->base = aligned_alloc(64, size);
    if (!a->base)
        return -1;

    a->size = size;
    a->offset = 0;

    return 0;
}

void *img_arena_alloc(img_arena_t *a, size_t size)
{
    if (!a)
        return NULL;

    size = align16(size);

    if (a->offset + size > a->size)
        return NULL;

    void *ptr = a->base + a->offset;
    a->offset += size;

    return ptr;
}

void img_arena_reset(img_arena_t *a)
{
    if (a)
        a->offset = 0;
}

void img_arena_destroy(img_arena_t *a)
{
    if (!a)
        return;

    free(a->base);
}