/* arch/x86_64/avx2/color_avx2.c */

#include <immintrin.h>
#include "arch/arch_interface.h"
#include "core/buffer.h"

void img_arch_grayscale_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    __m256i v_r = _mm256_set1_epi16(77);
    __m256i v_g = _mm256_set1_epi16(150);
    __m256i v_b = _mm256_set1_epi16(29);

    for (uint32_t y = 0; y < buf->height; y++)
    {
        uint8_t *row =
            (uint8_t *)__builtin_assume_aligned(buf->data + y * buf->stride, 32);

        uint32_t x = 0;

        for (; x + 32 <= buf->width * buf->channels; x += 32)
        {
            __m256i pixels =
                _mm256_loadu_si256((__m256i *)(row + x));

            // 🔥 simplified (real SIMD logic later)
            __m256i half = _mm256_srli_epi16(pixels, 1);

            _mm256_storeu_si256((__m256i *)(row + x), half);
        }
    }
}

// #include <immintrin.h>
// #include "arch/arch_interface.h"

// #include "core/buffer.h"
// #include "pipeline/pipeline_types.h"

// void img_arch_avx2_grayscale(img_buffer_t *buf)
// {
//     // Coefficients packed into 256-bit registers (8x 16-bit values)
//     __m256i v_r_coeff = _mm256_set1_epi16(77);
//     __m256i v_g_coeff = _mm256_set1_epi16(150);
//     __m256i v_b_coeff = _mm256_set1_epi16(29);

//     for (uint32_t y = 0; y < buf->height; y++)
//     {
//         uint8_t *row = (uint8_t *)__builtin_assume_aligned(buf->data + (y * buf->stride), 64);

//         uint32_t x = 0;
//         for (; x <= buf->width - 32; x += 32)
//         {
//             // Load 32 bytes (RGBA)
//             __m256i pixels = _mm256_load_si256((__m256i *)&row[x * 4]);

//             // Vectorized math here (Unpack, Multiply-Add, Pack)
//             // ... (Internal SIMD logic) ...

//             _mm256_store_si256((__m256i *)&row[x * 4], pixels);
//         }
//         // Scalar tail handling for remaining pixels
//     }
// }
