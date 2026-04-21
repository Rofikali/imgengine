// ./src/arch/x86_64/avx2/blit_avx2.c
// src/arch/x86_64/avx2/blit_avx2.c

// AVX2 blit: copy img_buffer_t onto canvas at (x, y).
// Hot path with no heap traffic on the common case.

#include <immintrin.h>
#include <string.h>
#include "core/buffer.h"

/*
 * img_blit_avx2()
 *
 * Blit src buffer onto dst canvas at pixel offset (dst_x, dst_y).
 * src must be RGB (3 channels), dst must be RGB (3 channels).
 *
 * HOT PATH CONTRACT:
 *   - No allocation
 *   - No branches inside row loop (except tail)
 *   - Clips silently at canvas boundary
 */
void img_blit_avx2(
    img_buffer_t *dst,
    const img_buffer_t *src,
    uint32_t dst_x,
    uint32_t dst_y)
{
    if (!dst || !src || !dst->data || !src->data)
        return;

    /* clip height */
    uint32_t rows = src->height;
    if (dst_y + rows > dst->height)
        rows = dst->height - dst_y;

    /* clip width */
    uint32_t cols = src->width;
    if (dst_x + cols > dst->width)
        cols = dst->width - dst_x;

    const uint32_t ch = 3;
    const uint32_t row_bytes = cols * ch;
    const uint32_t src_stride = src->stride;
    const uint32_t dst_stride = dst->stride;

    for (uint32_t y = 0; y < rows; y++)
    {
        const uint8_t *src_row = src->data + (size_t)y * src_stride;
        uint8_t *dst_row = dst->data +
                           (size_t)(dst_y + y) * dst_stride +
                           (size_t)dst_x * ch;

        /* prefetch next source row */
        __builtin_prefetch(src_row + src_stride, 0, 1);

        uint32_t x = 0;

        /* 32-byte AVX2 copy */
        for (; x + 32 <= row_bytes; x += 32)
        {
            __m256i v = _mm256_loadu_si256((const __m256i *)(src_row + x));
            _mm256_storeu_si256((__m256i *)(dst_row + x), v);
        }

        /* scalar tail */
        for (; x < row_bytes; x++)
            dst_row[x] = src_row[x];
    }
}

/*
 * img_blit_scalar()
 *
 * Reference implementation. Ground truth.
 * Used on non-AVX2 CPUs.
 */
void img_blit_scalar(
    img_buffer_t *dst,
    const img_buffer_t *src,
    uint32_t dst_x,
    uint32_t dst_y)
{
    if (!dst || !src || !dst->data || !src->data)
        return;

    uint32_t rows = src->height;
    if (dst_y + rows > dst->height)
        rows = dst->height - dst_y;

    uint32_t cols = src->width;
    if (dst_x + cols > dst->width)
        cols = dst->width - dst_x;

    const uint32_t ch = 3;
    const uint32_t row_bytes = cols * ch;

    for (uint32_t y = 0; y < rows; y++)
    {
        const uint8_t *src_row = src->data + (size_t)y * src->stride;
        uint8_t *dst_row = dst->data +
                           (size_t)(dst_y + y) * dst->stride +
                           (size_t)dst_x * ch;
        memcpy(dst_row, src_row, row_bytes);
    }
}
