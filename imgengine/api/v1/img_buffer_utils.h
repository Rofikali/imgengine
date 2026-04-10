
// api/v1/img_buffer_utils.h

#ifndef IMGENGINE_IMG_BUFFER_UTILS_H
#define IMGENGINE_IMG_BUFFER_UTILS_H

#include "core/buffer.h"
#include "memory/slab.h"

/*
 * 🔥 Create buffer INSIDE slab (header + data contiguous)
 */

static inline img_buffer_t img_buffer_from_slab(
    img_slab_pool_t *pool,
    uint32_t w,
    uint32_t h,
    uint32_t ch)
{
    img_buffer_t buf = {0};

    if (!pool || w == 0 || h == 0 || ch == 0)
        return buf;

    // 🔥 overflow-safe
    if (w > SIZE_MAX / ch)
        return buf;

    size_t stride = w * ch;

    if (stride > SIZE_MAX / h)
        return buf;

    size_t data_size = stride * h;

    // 🔥 alignment
    size_t hdr_size = (sizeof(img_buf_header_t) + 63) & ~63;
    size_t total = hdr_size + data_size;

    // 🔥 slab contract check
    size_t block = img_slab_block_size(pool);
    if (total > block)
        return buf;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return buf;

    img_buf_header_t *hdr = (img_buf_header_t *)mem;
    hdr->ref = 1;
    hdr->flags = 0;

    buf.data = mem + hdr_size;
    buf.width = w;
    buf.height = h;
    buf.channels = ch;
    buf.stride = stride;

    return buf;
}

#endif
