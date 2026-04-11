// ./src/arch/x86_64/avx512/avx512.c

#include <immintrin.h>
#include "arch/arch_interface.h"
#include "plugins/plugin_resize.h"

#include "core/buffer.h"
#include "pipeline/pipeline_types.h"

void img_arch_resize_avx512(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    resize_params_t *p = (resize_params_t *)params;

    // Assume buf already points to destination memory
    // You can embed src pointer inside params if needed

    (void)ctx;

    // 🔥 SIMD LOGIC HERE (simplified)
    // for (...) use AVX512 instructions

    buf->width = p->target_w;
    buf->height = p->target_h;
}

/* FILE: src/arch/x86_64/avx512/avx512.c — add at bottom */

/*
 * resize_avx512: canonical name used by jump_table.c
 */
void resize_avx512(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    img_arch_resize_avx512(ctx, buf, params);
}