#include <immintrin.h>
#include "arch/arch_interface.h"

void resize_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    uint8_t *data = buf->data;
    uint32_t n = buf->width * buf->height * buf->channels;

    for (uint32_t i = 0; i < n; i += 32)
    {
        __m256i v = _mm256_loadu_si256((__m256i *)&data[i]);
        v = _mm256_srli_epi16(v, 1);
        _mm256_storeu_si256((__m256i *)&data[i], v);
    }
}