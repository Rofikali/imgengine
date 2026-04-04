// src/core/buffer_lifecycle.c

#include "runtime/buffer_lifecycle.h"
// #include "api/v1/img_types.h"
#include "core/buffer.h"
#include <stdatomic.h>

typedef struct
{
    _Atomic uint32_t ref;
} ref_counter_t;

void img_buffer_acquire(img_buffer_t *buf)
{
    if (!buf)
        return;

    ref_counter_t *rc = (ref_counter_t *)(buf->data - sizeof(ref_counter_t));
    atomic_fetch_add(&rc->ref, 1);
}

void img_buffer_release(img_buffer_t *buf)
{
    if (!buf)
        return;

    ref_counter_t *rc = (ref_counter_t *)(buf->data - sizeof(ref_counter_t));

    if (atomic_fetch_sub(&rc->ref, 1) == 1)
    {
        // 🔥 return to slab
        // ctx not available → design decision: global pool fallback
    }
}