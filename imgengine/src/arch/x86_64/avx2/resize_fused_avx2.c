// arch/x86_64/avx2/resize_fused_avx2.c

#include <immintrin.h>
#include "arch/arch_interface.h"
#include "pipeline/batch.h"
#include "plugins/plugin_resize.h"

#include "pipeline/pipeline_types.h"
#include "core/buffer.h"

void img_batch_resize_fused_avx2(
    img_ctx_t *ctx,
    void *batch_ptr,
    void *params)
{
    (void)ctx;

    img_batch_t *batch = (img_batch_t *)batch_ptr;
    resize_params_t *p = (resize_params_t *)params;

    for (uint32_t b = 0; b < batch->count; b++)
    {
        img_buffer_t *dst = batch->buffers[b];
        img_buffer_t *src = p->src;

        for (uint32_t y = 0; y < dst->height; y++)
        {
            uint32_t sy = p->y_index[y];

            uint8_t *src_row = src->data + sy * src->stride;
            uint8_t *dst_row = dst->data + y * dst->stride;

            uint32_t i = 0;
            uint32_t size = dst->width * dst->channels;

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
}