#ifndef IMGENGINE_ARENA_H
#define IMGENGINE_ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t *base;
    size_t size;
    size_t offset;

} img_arena_t;

int img_arena_init(img_arena_t *a, size_t size);
void img_arena_destroy(img_arena_t *a);

void *img_arena_alloc(img_arena_t *a, size_t size);
void img_arena_reset(img_arena_t *a);

#endif