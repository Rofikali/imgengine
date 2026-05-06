// ./src/memory/arena_alloc_aligned.c
#include "memory/align.h"
#include "memory/arena.h"

void *img_arena_alloc_aligned(img_arena_t *arena, size_t size, size_t align) {
    if (!arena || align == 0)
        return NULL;

    size_t aligned_offset = img_align_up(arena->offset, align);
    if (aligned_offset + size > arena->size)
        return NULL;

    void *ptr = arena->base + aligned_offset;
    arena->offset = aligned_offset + size;
    return ptr;
}
