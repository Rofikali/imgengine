/* arch/x86_64/scalar/color_scalar.c */
#include "arch/arch_interface.h"
#include "core/buffer.h"
#include "pipeline/pipeline_types.h"


void img_arch_scalar_grayscale(img_buffer_t *buf)
{
    for (uint32_t y = 0; y < buf->height; y++)
    {
        uint8_t *row = buf->data + (y * buf->stride);
        for (uint32_t x = 0; x < buf->width; x++)
        {
            uint32_t i = x * buf->channels;
            // Luminance: 0.299R + 0.587G + 0.114B
            uint8_t g = (uint8_t)((row[i] * 77 + row[i + 1] * 150 + row[i + 2] * 29) >> 8);
            row[i] = row[i + 1] = row[i + 2] = g;
        }
    }
}
