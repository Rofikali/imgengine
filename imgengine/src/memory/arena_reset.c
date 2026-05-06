// ./src/memory/arena_reset.c
#include "memory/arena.h"

void img_arena_reset(img_arena_t *arena) {
    if (arena)
        arena->offset = 0;
}
