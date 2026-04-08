/* arch/x86_64/scalar/color_scalar.c */

#include "arch/arch_interface.h"
#include "core/buffer.h"

void img_arch_grayscale_scalar(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    for (uint32_t y = 0; y < buf->height; y++)
    {
        uint8_t *row = buf->data + (y * buf->stride);

        for (uint32_t x = 0; x < buf->width; x++)
        {
            uint32_t i = x * buf->channels;

            uint8_t g = (uint8_t)((row[i] * 77 + row[i + 1] * 150 + row[i + 2] * 29) >> 8);

            row[i] = row[i + 1] = row[i + 2] = g;
        }
    }
}
