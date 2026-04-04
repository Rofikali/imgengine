// plugins/plugin_grayscale.c

#include "plugins/plugin_internal.h"
#include "api/v1/img_types.h"

void plugin_grayscale_single(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    if (!buf)
        return;

    uint8_t *data = buf->data;

    for (uint32_t y = 0; y < buf->height; y++)
    {
        for (uint32_t x = 0; x < buf->width; x++)
        {
            uint8_t *px = data + y * buf->stride + x * buf->channels;

            uint8_t gray = (uint8_t)((px[0] * 77 + px[1] * 150 + px[2] * 29) >> 8);

            px[0] = px[1] = px[2] = gray;
        }
    }
}