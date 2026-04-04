

// arch/x86_64/scalar/resize_scalar.c
#include "core/buffer.h"
#include "pipeline/pipeline_types.h"
#include "plugins/plugin_resize.h"
#include "plugins/plugin_internal.h"
#include <string.h>

void img_arch_resize_h_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params)
{
    (void)ctx;
    resize_params_t *p = (resize_params_t *)params;

    for (uint32_t y = 0; y < p->src->height; y++)
    {
        uint8_t *src_row = p->src->data + y * p->src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        for (uint32_t x = 0; x < dst->width; x++)
        {
            uint32_t sx = p->x_index[x];

            for (uint32_t c = 0; c < p->src->channels; c++)
            {
                dst_row[x * p->src->channels + c] =
                    src_row[sx * p->src->channels + c];
            }
        }
    }
}

void img_arch_resize_v_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params)
{
    (void)ctx;
    resize_params_t *p = (resize_params_t *)params;

    for (uint32_t y = 0; y < dst->height; y++)
    {
        uint32_t sy = p->y_index[y];

        uint8_t *src_row = p->src->data + sy * p->src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        memcpy(dst_row, src_row, dst->width * dst->channels);
    }
}