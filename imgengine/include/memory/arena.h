// ./include/memory/arena.h

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

// ================= LIFECYCLE =================

img_arena_t *img_arena_create(size_t size);
void img_arena_destroy(img_arena_t *arena);

// ================= ALLOC =================

// 🔥 aligned allocation (cache-line safe)
void *img_arena_alloc_aligned(
    img_arena_t *arena,
    size_t size,
    size_t align);

// 🔥 default (16-byte aligned)
void *img_arena_alloc(img_arena_t *arena, size_t size);

// 🔥 reset (O(1))
void img_arena_reset(img_arena_t *arena);

#endif