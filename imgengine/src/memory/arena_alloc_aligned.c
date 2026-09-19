// ./src/memory/arena_alloc_aligned.c
#include "memory/align.h"
#include "memory/arena.h"

#include <stdint.h>

void *img_arena_alloc_aligned(img_arena_t *arena, size_t size, size_t align) {
    if (!arena || !arena->base || align == 0 || (align & (align - 1u)) != 0 ||
        arena->offset > arena->size || arena->offset > SIZE_MAX - (align - 1u))
        return NULL;

    size_t aligned_offset = img_align_up(arena->offset, align);
    if (aligned_offset > arena->size || size > arena->size - aligned_offset)
        return NULL;

    void *ptr = arena->base + aligned_offset;
    arena->offset = aligned_offset + size;
    return ptr;
}
