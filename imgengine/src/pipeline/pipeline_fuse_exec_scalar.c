// ./src/pipeline/pipeline_fuse_exec_scalar.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_exec_internal.h"
#include "pipeline/fused_params.h"

#include <stddef.h>

void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf) {
    const img_fused_params_t *p = (const img_fused_params_t *)ctx->fused_params;

    const uint32_t w = buf->width;
    const uint32_t h = buf->height;
    const uint32_t ch = buf->channels;

    for (uint32_t y = 0; y < h; y++) {
        uint8_t *row = buf->data + (size_t)y * buf->stride;
        for (uint32_t x = 0; x < w; x++) {
            uint8_t *pixel = row + x * ch;
            if (p->has_grayscale) {
                uint8_t gray = (uint8_t)(((uint32_t)pixel[0] * 77u + (uint32_t)pixel[1] * 150u +
                                          (uint32_t)pixel[2] * 29u) >>
                                         8);
                pixel[0] = gray;
                pixel[1] = gray;
                pixel[2] = gray;
            }
            if (p->has_brightness) {
                for (uint32_t channel = 0; channel < ch; channel++) {
                    uint32_t value = (uint32_t)pixel[channel] + p->brightness_value;
                    pixel[channel] = value > 255u ? 255u : (uint8_t)value;
                }
            }
        }
    }
}
