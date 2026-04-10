
// src/core/buffer_lifecycle.c

#include "runtime/buffer_lifecycle.h"
#include "core/buffer.h"

void img_buffer_acquire(img_buffer_t *buf)
{
    if (!buf || !buf->data)
        return;

    img_buf_header_t *hdr = img_buf_hdr(buf);
    atomic_fetch_add_explicit(&hdr->ref, 1, memory_order_relaxed);
}

void img_buffer_release(img_buffer_t *buf)
{
    if (!buf || !buf->data)
        return;

    img_buf_header_t *hdr = img_buf_hdr(buf);

    if (atomic_fetch_sub_explicit(&hdr->ref, 1, memory_order_acq_rel) == 1)
    {
        // 🔥 RETURN TO SLAB (future hook)
        // For now: NO-OP (ownership handled by pipeline)
    }
}
