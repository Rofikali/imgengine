#ifndef IMGENGINE_BUFFER_H
#define IMGENGINE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

#define IMG_LIKELY(x) __builtin_expect(!!(x), 1)
#define IMG_UNLIKELY(x) __builtin_expect(!!(x), 0)

/*
 * 🔥 INTERNAL HEADER
 */
typedef struct
{
    uint32_t ref;
    uint32_t flags;

} img_buf_header_t;

/*
 * 🔥 PUBLIC BUFFER
 */
typedef struct img_buffer
{
    uint8_t *data;

    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t stride;

} img_buffer_t;

/*
 * 🔥 SAFE SIZE CALCULATION
 */
static inline int img_buffer_size(
    const img_buffer_t *buf,
    size_t *out)
{
    if (IMG_UNLIKELY(!buf || !out))
        return -1;

    if (IMG_UNLIKELY(buf->width == 0 ||
                     buf->height == 0 ||
                     buf->channels == 0))
        return -1;

    size_t stride = (size_t)buf->width * buf->channels;

    if (IMG_UNLIKELY(stride / buf->channels != buf->width))
        return -1;

    size_t total = stride * buf->height;

    if (IMG_UNLIKELY(total / stride != buf->height))
        return -1;

    *out = total;
    return 0;
}

#endif