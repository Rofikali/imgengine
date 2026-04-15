// ./src/memory/arena.c

#include "memory/poison.h"
#include "memory/arena.h"
#include <stdlib.h>

static inline size_t align64(size_t x)
{
    return (x + 63) & ~63;
}

img_arena_t *img_arena_create(size_t size)
{
    img_arena_t *arena = malloc(sizeof(img_arena_t));
    if (!arena)
        return NULL;

    size = align64(size);

    arena->base = aligned_alloc(64, size);
    if (!arena->base)
    {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->offset = 0;

    return arena;
}

void *img_arena_alloc(img_arena_t *arena, size_t size)
{
    if (!arena)
        return NULL;

    size = align64(size);

    if (arena->offset + size > arena->size)
        return NULL;

    void *ptr = arena->base + arena->offset;
    arena->offset += size;

    return ptr;
}

void *img_arena_alloc_aligned(img_arena_t *arena, size_t size, size_t align)
{
    if (!arena || align == 0)
        return NULL;

    size_t aligned_offset = (arena->offset + align - 1) & ~(align - 1);

    if (aligned_offset + size > arena->size)
        return NULL;

    void *ptr = arena->base + aligned_offset;
    arena->offset = aligned_offset + size;

    return ptr;
}

void img_arena_reset(img_arena_t *arena)
{
    if (arena)
        arena->offset = 0;
}

void img_arena_destroy(img_arena_t *arena)
{
    if (!arena)
        return;

    free(arena->base);
    free(arena);
}
