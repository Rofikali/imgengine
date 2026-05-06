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
        for (uint32_t x = 0; x < w * ch; x++) {
            uint8_t px = row[x];
            if (p->has_grayscale)
                px = (uint8_t)((px * 77u) >> 8);
            if (p->has_brightness) {
                uint32_t tmp = (uint32_t)px + p->brightness_value;
                px = (tmp > 255u) ? 255u : (uint8_t)tmp;
            }
            row[x] = px;
        }
    }
}
