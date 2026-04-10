
// include/pipeline/fused_params.h

#ifndef IMGENGINE_FUSED_PARAMS_H
#define IMGENGINE_FUSED_PARAMS_H

#include <stdint.h>

typedef struct
{
    uint8_t has_grayscale;
    uint8_t has_brightness;
    uint8_t has_resize;

    uint16_t brightness_value;
    uint32_t new_w;
    uint32_t new_h;

} img_fused_params_t;

#endif