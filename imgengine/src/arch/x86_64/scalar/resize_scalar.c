// ./src/arch/x86_64/scalar/resize_scalar.c

#include "arch/arch_interface.h"
#include "core/buffer.h"
// #include "plugins/plugin_resize.h"
#include "arch/resize_params.h"

#include <string.h>

static inline uint32_t img_resize_axis_index(uint32_t pos, uint32_t scale, uint32_t limit) {
    uint32_t idx = (uint32_t)(((uint64_t)pos * (uint64_t)scale) >> 16);
    return (idx + 1 < limit) ? idx : (limit - 1);
}

static inline uint16_t img_resize_axis_weight(uint32_t pos, uint32_t scale) {
    return (uint16_t)(((uint64_t)pos * (uint64_t)scale) & 0xffffu);
}

/*
 * 🔥 MAIN RESIZE ENTRY (KERNEL ABI)
 */
void resize_scalar(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src || !buf || !buf->data)
        return;

    const img_buffer_t *src = p->src;
    const uint32_t src_w = src->width;
    const uint32_t src_h = src->height;
    const uint32_t ch = 3;
    const uint32_t dst_w = buf->width ? buf->width : p->target_w;
    const uint32_t dst_h = buf->height ? buf->height : p->target_h;

    if (dst_w == 0 || dst_h == 0 || src_w == 0 || src_h == 0)
        return;

    const uint32_t x_ratio = p->scale_x ? p->scale_x
                             : (dst_w > 1 && src_w > 1)
                                 ? (uint32_t)(((uint64_t)(src_w - 1) << 16) / (dst_w - 1))
                                 : 0;
    const uint32_t y_ratio = p->scale_y ? p->scale_y
                             : (dst_h > 1 && src_h > 1)
                                 ? (uint32_t)(((uint64_t)(src_h - 1) << 16) / (dst_h - 1))
                                 : 0;

    for (uint32_t dy = 0; dy < dst_h; dy++) {
        uint32_t y0 = p->y_index ? p->y_index[dy] : img_resize_axis_index(dy, y_ratio, src_h);
        uint32_t fy = p->y_weight ? p->y_weight[dy] : img_resize_axis_weight(dy, y_ratio);
        uint32_t y1 = (y0 + 1 < src_h) ? y0 + 1 : y0;
        uint32_t wy0 = 65536u - fy;

        const uint8_t *row0 = src->data + (size_t)y0 * src->stride;
        const uint8_t *row1 = src->data + (size_t)y1 * src->stride;
        uint8_t *dst_row = buf->data + (size_t)dy * buf->stride;

        for (uint32_t dx = 0; dx < dst_w; dx++) {
            uint32_t x0 = p->x_index ? p->x_index[dx] : img_resize_axis_index(dx, x_ratio, src_w);
            uint32_t fx = p->x_weight ? p->x_weight[dx] : img_resize_axis_weight(dx, x_ratio);
            uint32_t x1 = (x0 + 1 < src_w) ? x0 + 1 : x0;
            uint32_t wx0 = 65536u - fx;

            const uint8_t *p00 = row0 + (size_t)x0 * ch;
            const uint8_t *p10 = row0 + (size_t)x1 * ch;
            const uint8_t *p01 = row1 + (size_t)x0 * ch;
            const uint8_t *p11 = row1 + (size_t)x1 * ch;
            uint8_t *out = dst_row + (size_t)dx * ch;

            for (uint32_t c = 0; c < ch; c++) {
                uint64_t top = (uint64_t)p00[c] * wx0 + (uint64_t)p10[c] * fx;
                uint64_t bottom = (uint64_t)p01[c] * wx0 + (uint64_t)p11[c] * fx;
                uint64_t value = top * wy0 + bottom * fy;
                out[c] = (uint8_t)((value + (1ULL << 31)) >> 32);
            }
        }
    }

    buf->width = dst_w;
    buf->height = dst_h;
    buf->channels = ch;
}

/*
 * 🔥 HORIZONTAL PASS
 */
void img_arch_resize_h_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src || !p->x_index)
        return;

    img_buffer_t *src = p->src;

    for (uint32_t y = 0; y < src->height; y++) {
        uint8_t *src_row = src->data + y * src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        for (uint32_t x = 0; x < dst->width; x++) {
            uint32_t sx = p->x_index[x];

            for (uint32_t c = 0; c < src->channels; c++) {
                dst_row[x * src->channels + c] = src_row[sx * src->channels + c];
            }
        }
    }
}

/*
 * 🔥 VERTICAL PASS
 */
void img_arch_resize_v_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src || !p->y_index)
        return;

    img_buffer_t *src = p->src;

    for (uint32_t y = 0; y < dst->height; y++) {
        uint32_t sy = p->y_index[y];

        uint8_t *src_row = src->data + sy * src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        memcpy(dst_row, src_row, dst->width * dst->channels);
    }
}
