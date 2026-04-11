// ./src/memory/arena.c

// ./src/memory/arena.c

#include "security/poision.h"
#include "memory/arena.h"
#include <stdlib.h>

static inline size_t align64(size_t x)
{
    return (x + 63) & ~63;
}

static void free_block(void *ptr, size_t size)
{
    IMG_POISON_MEMORY(ptr, size);
}

img_arena_t *img_arena_create(size_t size)
{

    // p->x_index = img_arena_alloc(ctx->scratch_arena,
    //                              sizeof(uint32_t) * p->target_w);

    // p->y_index = img_arena_alloc(ctx->scratch_arena,
    //                              sizeof(uint32_t) * p->target_h);

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
