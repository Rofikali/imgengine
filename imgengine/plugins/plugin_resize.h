/* plugins/plugin_resize.h */

#ifndef IMGENGINE_PLUGIN_RESIZE_H
#define IMGENGINE_PLUGIN_RESIZE_H

#include <stdint.h>

// 🔥 Forward declarations (NO heavy includes)
typedef struct img_buffer img_buffer_t;
typedef struct img_ctx img_ctx_t;

typedef struct
{
    img_buffer_t *src;

    uint32_t target_w;
    uint32_t target_h;

    uint32_t scale_x;
    uint32_t scale_y;

    uint32_t *x_index;
    uint32_t *y_index;

} resize_params_t;

void plugin_resize_single(img_ctx_t *, img_buffer_t *, void *);

#endif
