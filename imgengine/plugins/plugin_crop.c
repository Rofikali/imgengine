// plugins/plugin_crop.c

#include "plugins/plugin_crop.h"
#include "api/v1/img_types.h"

typedef struct
{
    uint32_t x, y, w, h;
} crop_params_t;

void plugin_crop_single(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;

    if (!buf || !params)
        return;

    crop_params_t *p = (crop_params_t *)params;

    // Bounds check
    if (p->x + p->w > buf->width ||
        p->y + p->h > buf->height)
        return;

    // 🔥 ZERO-COPY slice
    buf->data += (p->y * buf->stride) + (p->x * buf->channels);
    buf->width = p->w;
    buf->height = p->h;
}
