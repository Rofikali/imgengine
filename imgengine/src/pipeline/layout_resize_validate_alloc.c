// ./src/pipeline/layout_resize_validate_alloc.c
#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"
#include "memory/arena.h"

#include <stddef.h>

img_result_t img_layout_resize_validate_alloc(
    img_ctx_t *ctx,
    const img_buffer_t *src,
    img_buffer_t *dst,
    uint32_t new_w,
    uint32_t new_h,
    img_slab_pool_t *pool,
    uint8_t **mem_out,
    uint32_t *stride_out)
{
    if (!src || !dst || !pool || !src->data || !mem_out || !stride_out)
        return IMG_ERR_SECURITY;
    if (new_w == 0 || new_h == 0)
        return IMG_ERR_SECURITY;
    if (src->width == 0 || src->height == 0 || src->channels < 3)
        return IMG_ERR_FORMAT;

    const uint32_t ch = 3;
    if (new_w > SIZE_MAX / ch)
        return IMG_ERR_NOMEM;

    size_t row_bytes = (size_t)new_w * ch;
    if (new_h > SIZE_MAX / row_bytes)
        return IMG_ERR_NOMEM;

    size_t required = row_bytes * new_h;

    /* Prefer scratch arena from ctx for temporary allocations when available */
    if (ctx && ctx->scratch_arena)
    {
        img_arena_t *arena = ctx->scratch_arena;
        /* ensure it fits and also not exceed typical slab block size */
        if (required <= arena->size - arena->offset && required <= img_slab_block_size(pool))
        {
            void *m = img_arena_alloc_aligned(arena, required, 64);
            if (m)
            {
                *mem_out = (uint8_t *)m;
                *stride_out = (uint32_t)row_bytes;
                /* mark owner as NULL — caller must not recycle via slab */
                dst->owner_pool = NULL;
                return IMG_SUCCESS;
            }
        }
    }

    if (row_bytes > UINT32_MAX || required > img_slab_block_size(pool))
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    *mem_out = mem;
    *stride_out = (uint32_t)row_bytes;
    dst->owner_pool = pool;
    return IMG_SUCCESS;
}
