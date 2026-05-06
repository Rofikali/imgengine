// ./src/pipeline/pipeline_fuse.c
#define _GNU_SOURCE

#include <stddef.h>
#include <string.h>

#include "pipeline/pipeline_fused.h"
#include "pipeline/pipeline_fuse_internal.h"
#include "pipeline/pipeline_signature.h"
#include "pipeline/pipeline_types.h"
#include "core/opcodes.h"

int img_pipeline_fuse(const img_pipeline_desc_t *in, img_pipeline_fused_t *out) {
    if (__builtin_expect(!in || !out, 0))
        return -1;

    __builtin_memset(&g_fused_params, 0, sizeof(g_fused_params));

    img_pipeline_sig_t sig = 0;

    for (uint32_t i = 0; i < in->count; i++) {
        const uint32_t op = in->ops[i].op_code;

        switch (op) {
        case OP_GRAYSCALE:
            g_fused_params.has_grayscale = 1;
            sig |= SIG_OP_GRAYSCALE;
            break;
        case OP_BRIGHTNESS:
            g_fused_params.has_brightness = 1;
            if (__builtin_expect(in->ops[i].params != NULL, 1))
                g_fused_params.brightness_value = *(const uint16_t *)in->ops[i].params;
            sig |= SIG_OP_BRIGHTNESS;
            break;
        case OP_RESIZE:
            g_fused_params.has_resize = 1;
            if (__builtin_expect(in->ops[i].params != NULL, 1)) {
                const uint32_t *dims = (const uint32_t *)in->ops[i].params;
                g_fused_params.new_w = dims[0];
                g_fused_params.new_h = dims[1];
            }
            sig |= SIG_OP_RESIZE;
            break;
        default:
            break;
        }
    }

    out->count = in->count;
    out->fn_batch = img_get_fused_kernel(sig);
    out->fn = img_pipeline_fuse_select_single_fn();

    return 0;
}
// static void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf)
// {
//     const img_fused_params_t *p =
//         (const img_fused_params_t *)ctx->fused_params;

//     const uint32_t w = buf->width;
//     const uint32_t h = buf->height;
//     const uint32_t ch = buf->channels;
//     uint8_t *const data = buf->data;

//     const __m256i zero = _mm256_setzero_si256();
//     const __m256i gray_coeff = _mm256_set1_epi16(77);
//     const __m256i bright_v = _mm256_set1_epi16(p->brightness_value);

//     for (uint32_t y = 0; y < h; y++)
//     {
//         uint8_t *row = data + (size_t)y * buf->stride;
//         const uint32_t len = w * ch;
//         uint32_t x = 0;

//         /* prefetch next row */
//         __builtin_prefetch(row + buf->stride, 0, 1);

//         for (; x + 32 <= len; x += 32)
//         {
//             __m256i v = _mm256_loadu_si256((const __m256i *)(row + x));
//             __m256i lo = _mm256_unpacklo_epi8(v, zero);
//             __m256i hi = _mm256_unpackhi_epi8(v, zero);

//             if (p->has_grayscale)
//             {
//                 lo = _mm256_srli_epi16(_mm256_mullo_epi16(lo, gray_coeff), 8);
//                 hi = _mm256_srli_epi16(_mm256_mullo_epi16(hi, gray_coeff), 8);
//             }

//             if (p->has_brightness)
//             {
//                 lo = _mm256_adds_epu16(lo, bright_v);
//                 hi = _mm256_adds_epu16(hi, bright_v);
//             }

//             _mm256_storeu_si256((__m256i *)(row + x),
//                                 _mm256_packus_epi16(lo, hi));
//         }

//         /* scalar tail */
//         for (; x < len; x++)
//         {
//             uint8_t px = row[x];

//             if (p->has_grayscale)
//                 px = (uint8_t)((px * 77u) >> 8);

//             if (p->has_brightness)
//             {
//                 uint32_t tmp = (uint32_t)px + p->brightness_value;
//                 px = (tmp > 255u) ? 255u : (uint8_t)tmp;
//             }

//             row[x] = px;
//         }
//     }
// }
// #endif /* __AVX2__ */

// /* ================================================================
//  * SCALAR SINGLE-IMAGE EXEC
//  *
//  * Reference implementation. Ground truth.
//  * No SIMD, no architecture assumptions.
//  * Must produce identical output to AVX2 path.
//  * ================================================================ */
// static void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf)
// {
//     const img_fused_params_t *p =
//         (const img_fused_params_t *)ctx->fused_params;

//     const uint32_t w = buf->width;
//     const uint32_t h = buf->height;
//     const uint32_t ch = buf->channels;

//     for (uint32_t y = 0; y < h; y++)
//     {
//         uint8_t *row = buf->data + (size_t)y * buf->stride;

//         for (uint32_t x = 0; x < w * ch; x++)
//         {
//             uint8_t px = row[x];

//             if (p->has_grayscale)
//                 px = (uint8_t)((px * 77u) >> 8);

//             if (p->has_brightness)
//             {
//                 uint32_t tmp = (uint32_t)px + p->brightness_value;
//                 px = (tmp > 255u) ? 255u : (uint8_t)tmp;
//             }

//             row[x] = px;
//         }
//     }
// }
