/* plugins/plugin_resize.h */

#ifndef IMGENGINE_PLUGIN_RESIZE_H
#define IMGENGINE_PLUGIN_RESIZE_H
#define OP_RESIZE_H 0x10
#define OP_RESIZE_V 0x11

#include <stdint.h>
#include "api/v1/img_types.h"

typedef struct
{
    img_buffer_t *src;

    uint32_t target_w;
    uint32_t target_h;

    uint32_t scale_x;
    uint32_t scale_y;

    // 🔥 Precomputed index tables
    uint32_t *x_index;
    uint32_t *y_index;

} resize_params_t;

#endif
