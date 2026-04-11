// ./include/core/buffer.h

// include/core/buffer.h

#ifndef IMGENGINE_BUFFER_H
#define IMGENGINE_BUFFER_H

#include <stdint.h>
#include <stdatomic.h>

/*
 * 🔥 Metadata (placed BEFORE data pointer)
 */
typedef struct
{
    _Atomic uint32_t ref;
    uint32_t flags;
} img_buf_header_t;

/*
 * 🔥 Buffer (ZERO-COPY CONTRACT SAFE)
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
 * 🔥 Helper: get header safely
 */
static inline img_buf_header_t *img_buf_hdr(img_buffer_t *buf)
{
    return (img_buf_header_t *)(buf->data - sizeof(img_buf_header_t));
}

#endif

// #ifndef IMGENGINE_BUFFER_H
// #define IMGENGINE_BUFFER_H

// #include <stdint.h>

// typedef struct img_buffer
// {
//     uint8_t *data;

//     uint32_t width;
//     uint32_t height;
//     uint32_t channels;
//     uint32_t stride;

// } img_buffer_t;

// #endif