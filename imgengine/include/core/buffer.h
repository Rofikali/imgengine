// include/core/buffer.h

// #ifndef IMGENGINE_CORE_BUFFER_H
// #define IMGENGINE_CORE_BUFFER_H

// #include <stdint.h>

// typedef enum
// {
//     IMG_BUF_STACK = 0, // not owned
//     IMG_BUF_SLAB = 1,  // owned by slab
// } img_buf_owner_t;

// typedef struct
// {
//     uint32_t width;
//     uint32_t height;
//     uint32_t channels;
//     uint32_t stride;

//     uint8_t *data;

//     // 🔥 L8: ownership metadata
//     img_buf_owner_t owner;

// } img_buffer_t;

// #endif

#ifndef IMGENGINE_BUFFER_H
#define IMGENGINE_BUFFER_H

#include <stdint.h>

typedef struct img_buffer
{
    uint8_t *data;

    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t stride;

} img_buffer_t;

#endif