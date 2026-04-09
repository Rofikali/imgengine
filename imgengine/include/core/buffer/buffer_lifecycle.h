#ifndef IMGENGINE_BUFFER_LIFECYCLE_H
#define IMGENGINE_BUFFER_LIFECYCLE_H

#include "core/buffer/buffer.h"
#include "memory/slab/slab.h"

void img_buffer_release(img_buffer_t *buf);
/*
 * 🔥 CREATE BUFFER FROM SLAB
 */
int img_buffer_alloc(
    img_slab_pool_t *pool,
    uint32_t w,
    uint32_t h,
    uint32_t ch,
    img_buffer_t *out);

/*
 * 🔥 REFCOUNT
 */
void img_buffer_retain(img_buffer_t *buf);
void img_buffer_release(img_buffer_t *buf);

#endif