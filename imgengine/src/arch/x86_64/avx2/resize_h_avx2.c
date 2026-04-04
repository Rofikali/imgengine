// arch/x86_64/avx2/resize_h_avx2.c

#include <immintrin.h>
#include "plugins/plugin_resize.h"
#include "arch/arch_interface.h"

#include "core/buffer.h"
#include "pipeline/pipeline_types.h"

void img_arch_resize_h_avx2(img_ctx_t *ctx, img_buffer_t *dst, void *params)
{
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;
    img_buffer_t *src = p->src;

    for (uint32_t y = 0; y < src->height; y++)
    {
        uint8_t *src_row = src->data + y * src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        uint32_t x = 0;

        for (; x + 32 <= dst->width; x += 32)
        {
            uint32_t sx = p->x_index[x];

            __m256i pixels = _mm256_loadu_si256(
                (__m256i *)(src_row + sx * src->channels));

            _mm256_storeu_si256(
                (__m256i *)(dst_row + x * dst->channels),
                pixels);
        }

        for (; x < dst->width; x++)
        {
            uint32_t sx = p->x_index[x];

            for (uint32_t c = 0; c < src->channels; c++)
            {
                dst_row[x * src->channels + c] =
                    src_row[sx * src->channels + c];
            }
        }
    }
}