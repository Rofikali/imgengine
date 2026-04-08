#include "core/buffer/buffer_lifecycle.h"
#include "memory/slab/slab.h"
#include <string.h>

/*
 * 🔥 ALLOC (ZERO-COPY SLAB)
 */
int img_buffer_alloc(
    img_slab_pool_t *pool,
    uint32_t w,
    uint32_t h,
    uint32_t ch,
    img_buffer_t *out)
{
    if (!pool || !out || w == 0 || h == 0 || ch == 0)
        return -1;

    if (w > SIZE_MAX / ch)
        return -1;

    size_t stride = w * ch;

    if (stride > SIZE_MAX / h)
        return -1;

    size_t data_size = stride * h;

    size_t hdr_size = (sizeof(img_buf_header_t) + 63) & ~63;
    size_t total = hdr_size + data_size;

    if (total > img_slab_block_size(pool))
        return -1;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return -1;

    img_buf_header_t *hdr = (img_buf_header_t *)mem;
    hdr->ref = 1;
    hdr->flags = 0;

    out->data = mem + hdr_size;
    out->width = w;
    out->height = h;
    out->channels = ch;
    out->stride = stride;

    return 0;
}

/*
 * 🔥 RETAIN
 */
void img_buffer_retain(img_buffer_t *buf)
{
    if (!buf)
        return;

    img_buf_header_t *hdr =
        (img_buf_header_t *)((uint8_t *)buf->data - 64);

    __atomic_fetch_add(&hdr->ref, 1, __ATOMIC_RELAXED);
}

/*
 * 🔥 RELEASE
 */
void img_buffer_release(img_buffer_t *buf)
{
    if (!buf)
        return;

    img_buf_header_t *hdr =
        (img_buf_header_t *)((uint8_t *)buf->data - 64);

    if (__atomic_sub_fetch(&hdr->ref, 1, __ATOMIC_ACQ_REL) == 0)
    {
        img_slab_free(hdr);
    }
}

void img_buffer_release(img_buffer_t *buf)
{
    if (!buf || !buf->data)
        return;

    // 🔥 assume slab allocated
    img_slab_free(NULL, buf->data); // (engine pool passed later)
}