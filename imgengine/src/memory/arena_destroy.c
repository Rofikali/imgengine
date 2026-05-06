// ./src/memory/arena_destroy.c
#include "memory/arena.h"

#include <stdlib.h>

void img_arena_destroy(img_arena_t *arena) {
    if (!arena)
        return;

    free(arena->base);
    free(arena);
}
