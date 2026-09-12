// ./src/memory/arena_destroy.c
#include "memory/arena.h"

#include <stdlib.h>

#if defined(_WIN32)
#include <malloc.h>
#endif

void img_arena_destroy(img_arena_t *arena) {
    if (!arena)
        return;

#if defined(_WIN32)
    _aligned_free(arena->base);
#else
    free(arena->base);
#endif
    free(arena);
}
