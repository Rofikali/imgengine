// ./src/memory/arena_create.c
#include "memory/align.h"
#include "memory/arena.h"

#include <stdlib.h>

img_arena_t *img_arena_create(size_t size) {
    img_arena_t *arena = malloc(sizeof(img_arena_t));
    if (!arena)
        return NULL;

    size = img_align64(size);

    arena->base = aligned_alloc(64, size);
    if (!arena->base) {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->offset = 0;
    return arena;
}
