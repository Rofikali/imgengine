// arch/x86_64/avx2/fused_resize_color_norm_avx2.c

#include <immintrin.h>
#include "arch/arch_interface.h"
#include "pipeline/batch.h"
#include "core/buffer.h"
#include "pipeline/pipeline_types.h"

void img_fused_resize_color_norm_avx2(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *params)
{
    (void)ctx;
    (void)params;

    for (uint32_t b = 0; b < batch->count; b++)
    {
        img_buffer_t *buf = batch->buffers[b];

        uint8_t *data = buf->data;

        for (uint32_t y = 0; y < buf->height; y++)
        {
            uint8_t *row = data + y * buf->stride;

            uint32_t x = 0;

            for (; x + 32 <= buf->width * buf->channels; x += 32)
            {
                __m256i pixels = _mm256_loadu_si256(
                    (__m256i *)(row + x));

                // 🔥 FAKE FUSION (extend later)
                // grayscale + normalize simulation

                __m256i zero = _mm256_setzero_si256();

                pixels = _mm256_avg_epu8(pixels, zero);

                _mm256_storeu_si256(
                    (__m256i *)(row + x),
                    pixels);
            }

            for (; x < buf->width * buf->channels; x++)
            {
                row[x] = row[x] >> 1;
            }
        }
    }
}

// #include <immintrin.h>
// #include "arch/arch_interface.h"
// #include "plugins/plugin_resize.h"

// void img_kernel_fused_resize_color_norm_avx2(
//     img_ctx_t *ctx,
//     img_buffer_t *dst,
//     void *params)
// {
//     (void)ctx;

//     resize_params_t *p = (resize_params_t *)params;
//     img_buffer_t *src = p->src;

//     const float norm = 1.0f / 255.0f;

//     __m256 norm_vec = _mm256_set1_ps(norm);

//     for (uint32_t dy = 0; dy < dst->height; dy++)
//     {
//         uint32_t sy = p->y_index[dy];

//         uint8_t *src_row = src->data + sy * src->stride;
//         float *dst_row = (float *)(dst->data + dy * dst->stride);

//         uint32_t dx = 0;

//         for (; dx + 8 <= dst->width; dx += 8)
//         {
//             // 🔥 gather pixels (nearest neighbor)
//             uint32_t sx0 = p->x_index[dx + 0];
//             uint32_t sx1 = p->x_index[dx + 1];
//             uint32_t sx2 = p->x_index[dx + 2];
//             uint32_t sx3 = p->x_index[dx + 3];
//             uint32_t sx4 = p->x_index[dx + 4];
//             uint32_t sx5 = p->x_index[dx + 5];
//             uint32_t sx6 = p->x_index[dx + 6];
//             uint32_t sx7 = p->x_index[dx + 7];

//             // 🔥 load RGB manually (no gather in AVX2)
//             uint8_t *p0 = src_row + sx0 * 3;
//             uint8_t *p1 = src_row + sx1 * 3;
//             uint8_t *p2 = src_row + sx2 * 3;
//             uint8_t *p3 = src_row + sx3 * 3;
//             uint8_t *p4 = src_row + sx4 * 3;
//             uint8_t *p5 = src_row + sx5 * 3;
//             uint8_t *p6 = src_row + sx6 * 3;
//             uint8_t *p7 = src_row + sx7 * 3;

//             float r[8], g[8], b[8];

//             r[0] = p0[0];
//             g[0] = p0[1];
//             b[0] = p0[2];
//             r[1] = p1[0];
//             g[1] = p1[1];
//             b[1] = p1[2];
//             r[2] = p2[0];
//             g[2] = p2[1];
//             b[2] = p2[2];
//             r[3] = p3[0];
//             g[3] = p3[1];
//             b[3] = p3[2];
//             r[4] = p4[0];
//             g[4] = p4[1];
//             b[4] = p4[2];
//             r[5] = p5[0];
//             g[5] = p5[1];
//             b[5] = p5[2];
//             r[6] = p6[0];
//             g[6] = p6[1];
//             b[6] = p6[2];
//             r[7] = p7[0];
//             g[7] = p7[1];
//             b[7] = p7[2];

//             __m256 vr = _mm256_loadu_ps(r);
//             __m256 vg = _mm256_loadu_ps(g);
//             __m256 vb = _mm256_loadu_ps(b);

//             // 🔥 grayscale (approx)
//             __m256 gray =
//                 _mm256_add_ps(
//                     _mm256_mul_ps(vr, _mm256_set1_ps(0.299f)),
//                     _mm256_add_ps(
//                         _mm256_mul_ps(vg, _mm256_set1_ps(0.587f)),
//                         _mm256_mul_ps(vb, _mm256_set1_ps(0.114f))));

//             // 🔥 normalize
//             gray = _mm256_mul_ps(gray, norm_vec);

//             // 🔥 store
//             _mm256_storeu_ps(dst_row + dx, gray);
//         }

//         // 🔥 tail
//         for (; dx < dst->width; dx++)
//         {
//             uint32_t sx = p->x_index[dx];
//             uint8_t *pixel = src_row + sx * 3;

//             float g =
//                 pixel[0] * 0.299f +
//                 pixel[1] * 0.587f +
//                 pixel[2] * 0.114f;

//             dst_row[dx] = g * norm;
//         }
//     }
// }