// ./src/arch/x86_64/avx2/resize_v_avx2.c

#include <immintrin.h>
#include "plugins/plugin_resize.h"
#include "arch/arch_interface.h"
#include "core/buffer.h"
#include "pipeline/pipeline_types.h"

void img_arch_resize_v_avx2(img_ctx_t *ctx, img_buffer_t *dst, void *params)
{
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    for (uint32_t y = 0; y < dst->height; y++)
    {
        uint32_t sy = p->y_index[y];

        uint8_t *src_row = p->src->data + sy * p->src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        uint32_t size = dst->width * dst->channels;

        uint32_t i = 0;

        for (; i + 32 <= size; i += 32)
        {
            __m256i pixels = _mm256_loadu_si256(
                (__m256i *)(src_row + i));

            _mm256_storeu_si256(
                (__m256i *)(dst_row + i),
                pixels);
        }

        for (; i < size; i++)
            dst_row[i] = src_row[i];
    }
}