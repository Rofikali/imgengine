// src/core/lbit_avx2.c

#include "imgengine/image.h"
#include <immintrin.h>
#include <stdint.h>

void img_blit_avx2(img_t *dst, const img_t *src, int x, int y)
{
    const int channels = 3;
    const int row_bytes = src->width * channels;

    for (int row = 0; row < src->height; row++)
    {
        uint8_t *dst_ptr =
            dst->data + ((y + row) * dst->width + x) * channels;

        const uint8_t *src_ptr =
            src->data + (row * src->width) * channels;

        int i = 0;

        // =========================
        // AVX2 COPY (32 bytes)
        // =========================
        for (; i <= row_bytes - 32; i += 32)
        {
            __m256i chunk = _mm256_loadu_si256((const __m256i *)(src_ptr + i));
            _mm256_storeu_si256((__m256i *)(dst_ptr + i), chunk);
        }

        // =========================
        // TAIL COPY
        // =========================
        for (; i < row_bytes; i++)
        {
            dst_ptr[i] = src_ptr[i];
        }
    }
}