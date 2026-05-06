// ./src/memory/arena_alloc.c
#include "memory/align.h"
#include "memory/arena.h"

void *img_arena_alloc(img_arena_t *arena, size_t size) {
    if (!arena)
        return NULL;

    size = img_align64(size);

    if (arena->offset + size > arena->size)
        return NULL;

    void *ptr = arena->base + arena->offset;
    arena->offset += size;
    return ptr;
}
