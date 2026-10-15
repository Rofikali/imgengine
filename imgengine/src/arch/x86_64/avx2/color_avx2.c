// ./src/arch/x86_64/avx2/color_avx2.c
// src/arch/x86_64/avx2/color_avx2.c
//
// The AVX2 dispatch entry delegates to the scalar RGB-luma implementation.
//
// The preserved operation contract is:
//   gray = (R*77 + G*150 + B*29) >> 8   (BT.601 luma approximation)
//
// Input pixels are RGB interleaved (3 bytes per pixel).
// AVX2 processes 32 bytes at a time — 10 full RGB pixels + 2 partial.
// Partial pixels handled in scalar tail.
//
// Correctness is preserved through the scalar fallback.

#include "arch/arch_interface.h"

void img_arch_grayscale_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    /*
     * The previous byte-wise AVX2 approximation was not equivalent to the
     * RGB-luma contract. Preserve the selected dispatch entry while routing
     * it to the canonical implementation until a separately verified
     * RGB-deinterleaving SIMD kernel exists.
     */
    img_arch_grayscale_scalar(ctx, buf, params);
}

// // ./src/arch/x86_64/avx2/color_avx2.c

// #include <immintrin.h>
// #include "arch/arch_interface.h"
// #include "core/buffer.h"

// void img_arch_grayscale_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params)
// {
//     (void)ctx;
//     (void)params;

//     __m256i v_r = _mm256_set1_epi16(77);
//     __m256i v_g = _mm256_set1_epi16(150);
//     __m256i v_b = _mm256_set1_epi16(29);

//     for (uint32_t y = 0; y < buf->height; y++)
//     {
//         uint8_t *row =
//             (uint8_t *)__builtin_assume_aligned(buf->data + y * buf->stride, 32);

//         uint32_t x = 0;

//         for (; x + 32 <= buf->width * buf->channels; x += 32)
//         {
//             __m256i pixels =
//                 _mm256_loadu_si256((__m256i *)(row + x));

//             __m256i half = _mm256_srli_epi16(pixels, 1);

//             _mm256_storeu_si256((__m256i *)(row + x), half);
//         }
//     }
// }
