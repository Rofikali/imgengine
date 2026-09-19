// ./src/memory/arena_alloc.c
#include "memory/align.h"
#include "memory/arena.h"

#include <stdint.h>

void *img_arena_alloc(img_arena_t *arena, size_t size) {
    if (!arena)
        return NULL;

    if (!arena->base || size > SIZE_MAX - 63u)
        return NULL;

    size = img_align64(size);
    if (arena->offset > arena->size || size > arena->size - arena->offset)
        return NULL;

    void *ptr = arena->base + arena->offset;
    arena->offset += size;
    return ptr;
}
