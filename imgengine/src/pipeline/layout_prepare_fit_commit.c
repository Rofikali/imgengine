// ./src/pipeline/layout_prepare_fit_commit.c
#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"

#include <stddef.h>
#include <string.h>

img_result_t img_layout_prepare_fit_commit(
    const img_buffer_t *tmp,
    img_buffer_t *dst,
    img_slab_pool_t *pool,
    uint32_t cell_w,
    uint32_t cell_h,
    uint8_t bg_r,
    uint8_t bg_g,
    uint8_t bg_b)
{
    if (tmp->width > cell_w || tmp->height > cell_h)
        return IMG_ERR_SECURITY;

    size_t row_bytes = (size_t)cell_w * 3;
    if (row_bytes > UINT32_MAX)
        return IMG_ERR_NOMEM;
    if (cell_h > SIZE_MAX / row_bytes)
        return IMG_ERR_NOMEM;

    size_t required = row_bytes * cell_h;
    if (required > img_slab_block_size(pool))
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    if (bg_r == bg_g && bg_g == bg_b)
    {
        memset(mem, bg_r, required);
    }
    else
    {
        for (size_t i = 0; i < required; i += 3)
        {
            mem[i] = bg_r;
            mem[i + 1] = bg_g;
            mem[i + 2] = bg_b;
        }
    }

    dst->data = mem;
    dst->width = cell_w;
    dst->height = cell_h;
    dst->channels = 3;
    dst->stride = (uint32_t)row_bytes;

    uint32_t off_x = (cell_w - tmp->width) / 2;
    uint32_t off_y = (cell_h - tmp->height) / 2;
    for (uint32_t dy = 0; dy < tmp->height; dy++)
    {
        uint8_t *dst_row = mem + (off_y + dy) * dst->stride + off_x * 3;
        const uint8_t *src_row = tmp->data + dy * tmp->stride;
        memcpy(dst_row, src_row, tmp->width * 3);
    }

    return IMG_SUCCESS;
}
