// arch/x86_64/avx2/fused_resize_color_norm_avx2.c

#include <immintrin.h>
#include "arch/arch_interface.h"
#include "hot/batch_exec.h"
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
