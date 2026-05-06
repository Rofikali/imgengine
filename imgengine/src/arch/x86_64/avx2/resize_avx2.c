// ./src/arch/x86_64/avx2/resize_avx2.c

#include <immintrin.h>
#include <stddef.h>

#include "arch/arch_interface.h"
#include "arch/resize_params.h"
#include "core/buffer.h"

static inline uint32_t img_resize_floor_fp(uint32_t pos, uint32_t scale) {
    return (uint32_t)(((uint64_t)pos * (uint64_t)scale) >> 16);
}

static inline uint16_t img_resize_frac_fp(uint32_t pos, uint32_t scale) {
    return (uint16_t)(((uint64_t)pos * (uint64_t)scale) & 0xffffu);
}

static inline uint8_t img_resize_clamp_u8(int32_t v) {
    if (v < 0)
        return 0;
    if (v > 255)
        return 255;
    return (uint8_t)v;
}

static inline void img_resize_blend8_rgb(uint8_t *dst_row, const uint8_t *row0, const uint8_t *row1,
                                         const uint32_t *x_index, const uint16_t *x_weight,
                                         uint32_t x_ratio, uint32_t src_w, uint32_t base_dx,
                                         uint32_t fy, uint32_t wy0) {
    uint32_t sx0[8];
    uint32_t sx1[8];
    uint32_t wx0[8];
    uint32_t fx[8];

    for (uint32_t lane = 0; lane < 8; lane++) {
        uint32_t dx = base_dx + lane;
        uint32_t x0 = x_index ? x_index[dx] : img_resize_floor_fp(dx, x_ratio);
        uint32_t frac = x_weight ? x_weight[dx] : img_resize_frac_fp(dx, x_ratio);
        uint32_t x1 = (x0 + 1 < src_w) ? x0 + 1 : x0;

        sx0[lane] = x0;
        sx1[lane] = x1;
        wx0[lane] = 65536u - frac;
        fx[lane] = frac;
    }

    const __m256i rounding = _mm256_set1_epi32(0x8000);
    const __m256i fy_v = _mm256_set1_epi32((int32_t)fy);
    const __m256i wy0_v = _mm256_set1_epi32((int32_t)wy0);

    __m256i wx0_v =
        _mm256_setr_epi32((int32_t)wx0[0], (int32_t)wx0[1], (int32_t)wx0[2], (int32_t)wx0[3],
                          (int32_t)wx0[4], (int32_t)wx0[5], (int32_t)wx0[6], (int32_t)wx0[7]);
    __m256i fx_v =
        _mm256_setr_epi32((int32_t)fx[0], (int32_t)fx[1], (int32_t)fx[2], (int32_t)fx[3],
                          (int32_t)fx[4], (int32_t)fx[5], (int32_t)fx[6], (int32_t)fx[7]);

    for (uint32_t c = 0; c < 3; c++) {
        int32_t p00[8];
        int32_t p10[8];
        int32_t p01[8];
        int32_t p11[8];

        for (uint32_t lane = 0; lane < 8; lane++) {
            size_t i0 = (size_t)sx0[lane] * 3u + c;
            size_t i1 = (size_t)sx1[lane] * 3u + c;
            p00[lane] = row0[i0];
            p10[lane] = row0[i1];
            p01[lane] = row1[i0];
            p11[lane] = row1[i1];
        }

        __m256i p00_v =
            _mm256_setr_epi32(p00[0], p00[1], p00[2], p00[3], p00[4], p00[5], p00[6], p00[7]);
        __m256i p10_v =
            _mm256_setr_epi32(p10[0], p10[1], p10[2], p10[3], p10[4], p10[5], p10[6], p10[7]);
        __m256i p01_v =
            _mm256_setr_epi32(p01[0], p01[1], p01[2], p01[3], p01[4], p01[5], p01[6], p01[7]);
        __m256i p11_v =
            _mm256_setr_epi32(p11[0], p11[1], p11[2], p11[3], p11[4], p11[5], p11[6], p11[7]);

        __m256i top_v =
            _mm256_srli_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(p00_v, wx0_v),
                                                                _mm256_mullo_epi32(p10_v, fx_v)),
                                               rounding),
                              16);
        __m256i bottom_v =
            _mm256_srli_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(p01_v, wx0_v),
                                                                _mm256_mullo_epi32(p11_v, fx_v)),
                                               rounding),
                              16);

        __m256i out_v =
            _mm256_srli_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(top_v, wy0_v),
                                                                _mm256_mullo_epi32(bottom_v, fy_v)),
                                               rounding),
                              16);

        int32_t out_i[8];
        _mm256_storeu_si256((__m256i *)out_i, out_v);

        for (uint32_t lane = 0; lane < 8; lane++) {
            dst_row[(size_t)(base_dx + lane) * 3u + c] = img_resize_clamp_u8(out_i[lane]);
        }
    }
}

void img_arch_avx2_resize(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;
    if (!p || !p->src || !dst || !dst->data)
        return;

    img_buffer_t *src = p->src;
    const uint32_t src_w = src->width;
    const uint32_t src_h = src->height;
    const uint32_t dst_w = dst->width ? dst->width : p->target_w;
    const uint32_t dst_h = dst->height ? dst->height : p->target_h;
    const uint32_t ch = 3;

    if (src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0)
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
        uint32_t y0 = p->y_index ? p->y_index[dy] : img_resize_floor_fp(dy, y_ratio);
        uint32_t fy = p->y_weight ? p->y_weight[dy] : img_resize_frac_fp(dy, y_ratio);
        uint32_t y1 = (y0 + 1 < src_h) ? y0 + 1 : y0;
        uint32_t wy0 = 65536u - fy;

        const uint8_t *row0 = src->data + (size_t)y0 * src->stride;
        const uint8_t *row1 = src->data + (size_t)y1 * src->stride;
        uint8_t *dst_row = dst->data + (size_t)dy * dst->stride;

        __builtin_prefetch(row0 + src->stride, 0, 1);
        __builtin_prefetch(row1 + src->stride, 0, 1);

        uint32_t dx = 0;
        for (; dx + 8 <= dst_w; dx += 8) {
            img_resize_blend8_rgb(dst_row, row0, row1, p->x_index, p->x_weight, x_ratio, src_w, dx,
                                  fy, wy0);
        }

        for (; dx < dst_w; dx++) {
            uint32_t x0 = p->x_index ? p->x_index[dx] : img_resize_floor_fp(dx, x_ratio);
            uint32_t fx = p->x_weight ? p->x_weight[dx] : img_resize_frac_fp(dx, x_ratio);
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

    dst->width = dst_w;
    dst->height = dst_h;
    dst->channels = ch;
}

/*
 * resize_avx2()
 *
 * Canonical name registered in jump_table.c.
 * Thin wrapper around img_arch_avx2_resize().
 * With LTO enabled, this compiles to a direct call.
 */
void resize_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    img_arch_avx2_resize(ctx, buf, params);
}
