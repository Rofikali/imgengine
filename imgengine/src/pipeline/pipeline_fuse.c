// pipeline/pipeline_fused.c

#include "pipeline/pipeline_fused.h"
#include "pipeline/pipeline_types.h"
#include "core/opcodes.h"
#include "core/buffer.h"
#include "pipeline/fused_params.h"
#include "pipeline/pipeline_signature.h"
#include "runtime/task.h"

#if defined(__AVX2__)
#include <immintrin.h>
#endif

/*
 * 🔥 STATIC STORAGE (NO MALLOC EVER)
 */
static img_pipeline_fused_t g_fused;

/*
 * 🔥 FORWARD DECLARATIONS
 */
static void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf);

#if defined(__AVX2__)
static void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf);
#endif

/*
 * 🔥 FUSION COMPILER (PIPELINE → IR)
 */

int img_pipeline_fuse(
    const img_pipeline_desc_t *in,
    img_pipeline_fused_t *out)
{
    if (!in || !out)
        return -1;

    static img_fused_params_t params;

    __builtin_memset(&params, 0, sizeof(params));

    for (uint32_t i = 0; i < in->count; i++)
    {
        uint32_t op = in->ops[i].op_code;

        switch (op)
        {
        case OP_GRAYSCALE:
            params.has_grayscale = 1;
            break;

        case OP_BRIGHTNESS:
            params.has_brightness = 1;
            params.brightness_value =
                *(uint16_t *)in->ops[i].params;
            break;
        }
    }

    out->params = params;

#if defined(__AVX2__)
    out->fn = fused_exec_avx2;
#else
    out->fn = fused_exec_scalar;
#endif

    return 0;
}

#if defined(__AVX2__)
static void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf)
{
    img_fused_params_t *p =
        (img_fused_params_t *)ctx->fused_params;

    uint32_t w = buf->width;
    uint32_t h = buf->height;
    uint32_t ch = buf->channels;

    uint8_t *data = buf->data;

    const __m256i zero = _mm256_setzero_si256();

    __m256i bright = _mm256_set1_epi16(p->brightness_value);
    const __m256i gray_coeff = _mm256_set1_epi16(77);

    for (uint32_t y = 0; y < h; y++)
    {
        uint8_t *row = data + y * buf->stride;

        __builtin_prefetch(row + 64, 0, 1);

        uint32_t len = w * ch;
        uint32_t x = 0;

        for (; x + 32 <= len; x += 32)
        {
            __m256i v = _mm256_loadu_si256((__m256i *)(row + x));

            __m256i lo = _mm256_unpacklo_epi8(v, zero);
            __m256i hi = _mm256_unpackhi_epi8(v, zero);

            /*
             * 🔥 NO SWITCH — ONLY PREDICATED OPS
             */

            if (p->has_grayscale)
            {
                lo = _mm256_srli_epi16(
                    _mm256_mullo_epi16(lo, gray_coeff), 8);

                hi = _mm256_srli_epi16(
                    _mm256_mullo_epi16(hi, gray_coeff), 8);
            }

            if (p->has_brightness)
            {
                lo = _mm256_adds_epu16(lo, bright);
                hi = _mm256_adds_epu16(hi, bright);
            }

            __m256i out = _mm256_packus_epi16(lo, hi);

            _mm256_storeu_si256((__m256i *)(row + x), out);
        }

        /* scalar tail */
        for (; x < len; x++)
        {
            uint8_t px = row[x];

            if (p->has_grayscale)
                px = (px * 77) >> 8;

            if (p->has_brightness)
            {
                uint16_t tmp = px + p->brightness_value;
                px = (tmp > 255) ? 255 : tmp;
            }

            row[x] = px;
        }
    }
}

static void fused_exec_scalar(
    img_ctx_t *ctx,
    img_buffer_t *buf)
{
    (void)ctx;

    uint32_t w = buf->width;
    uint32_t h = buf->height;
    uint32_t ch = buf->channels;

    uint8_t *data = buf->data;

    for (uint32_t y = 0; y < h; y++)
    {
        uint8_t *row = data + y * buf->stride;

        for (uint32_t x = 0; x < w * ch; x++)
        {
            uint8_t px = row[x];

            for (uint32_t op = 0; op < g_fused.count; op++)
            {
                if (g_fused.ops[op] == FUSED_OP_GRAYSCALE)
                    px = (px * 77) >> 8;

                else if (g_fused.ops[op] == FUSED_OP_BRIGHTNESS)
                {
                    uint16_t tmp = px + 20;
                    px = (tmp > 255) ? 255 : tmp;
                }
            }

            row[x] = px;
        }
    }
}

// void img_pipeline_execute_fused(
//     img_ctx_t *ctx,
//     img_pipeline_fused_t *pipe,
//     img_buffer_t *buf)
// {
//     ctx->fused_params = &pipe->params;

//     pipe->fn(ctx, buf);
// }

int img_pipeline_fuse(
    const img_pipeline_desc_t *in,
    img_fused_params_t *params,
    img_pipeline_sig_t *sig_out)
{
    if (!in || !params || !sig_out)
        return -1;

    img_pipeline_sig_t sig = 0;

    for (uint32_t i = 0; i < in->count; i++)
    {
        switch (in->ops[i].op_code)
        {
        case OP_GRAYSCALE:
            params->has_grayscale = 1;
            sig |= SIG_OP_GRAYSCALE;
            break;

        case SIG_OP_BRIGHTNESS:
            params->has_brightness = 1;
            params->has_brightness =
                *(uint16_t *)in->ops[i].params;
            sig |= SIG_OP_BRIGHTNESS;
            break;

        case OP_RESIZE:
            params->has_resize = 1;
            params->new_w =
                ((uint32_t *)in->ops[i].params)[0];
            params->new_h =
                ((uint32_t *)in->ops[i].params)[1];
            sig |= SIG_OP_RESIZE;
            break;
        }
    }

    *sig_out = sig;
    return 0;
}

void img_pipeline_execute_fused_batch(
    img_ctx_t *ctx,
    img_pipeline_fused_t *pipe,
    img_batch_t *batch)
{
    if (!pipe || !batch)
        return;

    for (uint32_t i = 0; i < batch->count; i++)
    {
        pipe->fn(ctx, &batch->buffers[i]);
    }
}

#endif