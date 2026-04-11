// ./include/hot/pixel_ops.h

// src/hot/pixel_ops.h

#ifndef IMGENGINE_PIXEL_OPS_H
#define IMGENGINE_PIXEL_OPS_H

#include <stdint.h>

// inline = zero function call overhead

static inline uint8_t pixel_lerp(uint8_t a, uint8_t b, uint8_t w)
{
    return (uint8_t)(a + ((int16_t)w * (b - a) >> 8));
}

#endif