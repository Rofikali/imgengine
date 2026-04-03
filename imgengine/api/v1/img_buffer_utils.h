// #ifndef IMGENGINE_BUFFER_UTILS_H
// #define IMGENGINE_BUFFER_UTILS_H

// #include "api/v1/img_types.h"

// static inline img_buffer_t img_buffer_create(
//     uint8_t *data,
//     uint32_t w,
//     uint32_t h,
//     uint32_t ch)
// {
//     img_buffer_t buf;
//     buf.data = data;
//     buf.width = w;
//     buf.height = h;
//     buf.channels = ch;

//     // 64-byte aligned stride
//     buf.stride = ((w * ch + 63) / 64) * 64;

//     return buf;
// }

// #endif

#ifndef IMGENGINE_IMG_BUFFER_UTILS_H
#define IMGENGINE_IMG_BUFFER_UTILS_H

#include "api/v1/img_types.h"

static inline img_buffer_t img_buffer_create(
    uint8_t *data,
    uint32_t w,
    uint32_t h,
    uint32_t ch)
{
    img_buffer_t buf;
    buf.data = data;
    buf.width = w;
    buf.height = h;
    buf.channels = ch;
    buf.stride = w * ch;
    return buf;
}

#endif