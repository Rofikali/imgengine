// /* memory/arena.h */

#ifndef IMGENGINE_MEMORY_ARENA_H
#define IMGENGINE_MEMORY_ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct img_arena
{
    uint8_t *base;
    size_t size;
    size_t offset;
} img_arena_t;

img_arena_t *img_arena_create(size_t size);
void *img_arena_alloc(img_arena_t *arena, size_t size);
void img_arena_reset(img_arena_t *arena);
void img_arena_destroy(img_arena_t *arena);

#endif