// ./src/pipeline/pipeline_fuse_exec_scalar.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_exec_internal.h"
#include "pipeline/fused_params.h"
#include "arch/arch_interface.h"

#include <stddef.h>

void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf) {
    const img_fused_params_t *p = (const img_fused_params_t *)ctx->fused_params;

    if (p->has_grayscale)
        img_arch_grayscale_scalar(ctx, buf, NULL);

    if (!p->has_brightness)
        return;

    for (uint32_t y = 0; y < buf->height; y++) {
        uint8_t *row = buf->data + (size_t)y * buf->stride;
        for (uint32_t x = 0; x < buf->width * buf->channels; x++) {
            uint8_t px = row[x];
            uint32_t tmp = (uint32_t)px + p->brightness_value;
            px = (tmp > 255u) ? 255u : (uint8_t)tmp;
            row[x] = px;
        }
    }
}
