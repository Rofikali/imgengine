// ./src/memory/arena_create.c
#include "memory/align.h"
#include "memory/arena.h"

#include <stdlib.h>

#if defined(_WIN32)
#include <malloc.h>
#endif

img_arena_t *img_arena_create(size_t size) {
    if (size == 0 || size > SIZE_MAX - 63u)
        return NULL;

    img_arena_t *arena = malloc(sizeof(img_arena_t));
    if (!arena)
        return NULL;

    size = img_align64(size);

#if defined(_WIN32)
    arena->base = _aligned_malloc(size, 64);
#else
    arena->base = aligned_alloc(64, size);
#endif
    if (!arena->base) {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->offset = 0;
    return arena;
}
