// ./src/pipeline/pipeline_fuse_exec_avx2.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_exec_internal.h"
#include "pipeline/fused_params.h"

#include <stddef.h>

#if defined(__AVX2__)
#include <immintrin.h>

void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf) {
    const img_fused_params_t *p = (const img_fused_params_t *)ctx->fused_params;

    const uint32_t w = buf->width;
    const uint32_t h = buf->height;
    const uint32_t ch = buf->channels;
    uint8_t *const data = buf->data;

    const __m256i zero = _mm256_setzero_si256();
    const __m256i gray_coeff = _mm256_set1_epi16(77);
    const __m256i bright_v = _mm256_set1_epi16(p->brightness_value);

    for (uint32_t y = 0; y < h; y++) {
        uint8_t *row = data + (size_t)y * buf->stride;
        const uint32_t len = w * ch;
        uint32_t x = 0;

        __builtin_prefetch(row + buf->stride, 0, 1);

        for (; x + 32 <= len; x += 32) {
            __m256i v = _mm256_loadu_si256((const __m256i *)(row + x));
            __m256i lo = _mm256_unpacklo_epi8(v, zero);
            __m256i hi = _mm256_unpackhi_epi8(v, zero);

            if (p->has_grayscale) {
                lo = _mm256_srli_epi16(_mm256_mullo_epi16(lo, gray_coeff), 8);
                hi = _mm256_srli_epi16(_mm256_mullo_epi16(hi, gray_coeff), 8);
            }
            if (p->has_brightness) {
                lo = _mm256_adds_epu16(lo, bright_v);
                hi = _mm256_adds_epu16(hi, bright_v);
            }

            _mm256_storeu_si256((__m256i *)(row + x), _mm256_packus_epi16(lo, hi));
        }

        for (; x < len; x++) {
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
#else
void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf) {
    (void)ctx;
    (void)buf;
}
#endif
