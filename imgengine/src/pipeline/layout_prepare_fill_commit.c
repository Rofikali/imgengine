// ./src/pipeline/layout_prepare_fill_commit.c
#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"

#include <stddef.h>
#include <string.h>

img_result_t img_layout_prepare_fill_commit(const img_buffer_t *tmp, img_buffer_t *dst,
                                            img_slab_pool_t *pool, uint32_t cell_w,
                                            uint32_t cell_h) {
    if (tmp->width < cell_w || tmp->height < cell_h)
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

    uint32_t cx = (tmp->width - cell_w) / 2;
    uint32_t cy = (tmp->height - cell_h) / 2;

    dst->data = mem;
    dst->owner_pool = pool;
    dst->width = cell_w;
    dst->height = cell_h;
    dst->channels = 3;
    dst->stride = (uint32_t)row_bytes;

    for (uint32_t dy = 0; dy < cell_h; dy++) {
        const uint8_t *src_row = tmp->data + (cy + dy) * tmp->stride + cx * 3;
        uint8_t *dst_row = mem + dy * dst->stride;
        memcpy(dst_row, src_row, cell_w * 3);
    }

    return IMG_SUCCESS;
}
