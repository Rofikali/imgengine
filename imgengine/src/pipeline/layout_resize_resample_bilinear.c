
// ================================================================
// FILE 4: src/pipeline/layout_resize_resample_bilinear.c  (REPLACE)
// THE KEY FILE. Now takes ctx, dispatches to jump table.
// ================================================================

#define _GNU_SOURCE

#include "pipeline/layout_resize_internal.h"
#include "pipeline/jump_table.h"
#include "arch/resize_params.h"
#include "core/opcodes.h"
#include "memory/arena.h"

static void img_layout_resize_precompute_axes(
    uint32_t count,
    uint32_t scale,
    uint32_t *index_out,
    uint16_t *weight_out)
{
    for (uint32_t i = 0; i < count; i++)
    {
        uint64_t fp = (uint64_t)i * (uint64_t)scale;
        index_out[i] = (uint32_t)(fp >> 16);
        weight_out[i] = (uint16_t)(fp & 0xffffu);
    }
}

void img_layout_resize_resample_bilinear(
    img_ctx_t *ctx,
    const img_buffer_t *src,
    uint8_t *mem,
    uint32_t new_w,
    uint32_t new_h,
    uint32_t stride)
{
    /*
     * HOT PATH: dispatch to registered SIMD kernel.
     *
     * g_jump_table[OP_RESIZE] is set by hardware_registry.c to:
     *   img_arch_avx2_resize   (on AVX2/AVX-512 CPUs)
     *   img_arch_resize_scalar (fallback)
     *
     * The kernel writes into the pre-allocated dst buffer (mem).
     * We build a stack img_buffer_t pointing at mem so the kernel
     * has correct width/height/stride metadata to write into.
     *
     * The resize_params_t carries the source buffer pointer and
     * target dimensions. The kernel computes scale factors internally
     * from src->width/height and target_w/target_h.
     *
     * Cost: 1 table lookup + 1 indirect call + AVX2 kernel.
     * At 300 DPI for a 4.5×3.5cm photo:
     *   531×413 px → ~220K pixels × 3ch = 660KB
     *   AVX2: processes 32 bytes/cycle → ~20K cycles (~7µs)
     *   Scalar bilinear: ~1 pixel/cycle → ~660K cycles (~220µs)
     *   Speedup: ~30x
     */
    img_kernel_fn resize_fn = g_jump_table[OP_RESIZE];

    if (__builtin_expect(resize_fn != NULL, 1))
    {
        const uint32_t src_w = src->width;
        const uint32_t src_h = src->height;
        const uint32_t scale_x = (new_w > 1 && src_w > 1)
                                     ? (uint32_t)(((uint64_t)(src_w - 1) << 16) / (new_w - 1))
                                     : 0;
        const uint32_t scale_y = (new_h > 1 && src_h > 1)
                                     ? (uint32_t)(((uint64_t)(src_h - 1) << 16) / (new_h - 1))
                                     : 0;

        img_buffer_t dst_buf = {
            .data = mem,
            .width = new_w,
            .height = new_h,
            .channels = 3,
            .stride = stride,
        };

        img_arena_t *arena = ctx ? ctx->scratch_arena : NULL;
        uint32_t *x_index = NULL;
        uint32_t *y_index = NULL;
        uint16_t *x_weight = NULL;
        uint16_t *y_weight = NULL;

        if (arena)
        {
            x_index = img_arena_alloc_aligned(
                arena, (size_t)new_w * sizeof(uint32_t), 64);
            y_index = img_arena_alloc_aligned(
                arena, (size_t)new_h * sizeof(uint32_t), 64);
            x_weight = img_arena_alloc_aligned(
                arena, (size_t)new_w * sizeof(uint16_t), 64);
            y_weight = img_arena_alloc_aligned(
                arena, (size_t)new_h * sizeof(uint16_t), 64);
        }

        if (x_index && y_index && x_weight && y_weight)
        {
            img_layout_resize_precompute_axes(new_w, scale_x, x_index, x_weight);
            img_layout_resize_precompute_axes(new_h, scale_y, y_index, y_weight);
        }
        else
        {
            x_index = NULL;
            y_index = NULL;
            x_weight = NULL;
            y_weight = NULL;
        }

        resize_params_t params = {
            .src = (struct img_buffer *)src,
            .target_w = new_w,
            .target_h = new_h,
            .scale_x = scale_x,
            .scale_y = scale_y,
            .x_index = x_index,
            .y_index = y_index,
            .x_weight = x_weight,
            .y_weight = y_weight,
        };

        resize_fn(ctx, &dst_buf, &params);
        return;
    }

    /*
     * FALLBACK: scalar bilinear.
     * Runs when jump table has no entry for OP_RESIZE:
     *   - engine_init not called (should not happen in normal flow)
     *   - running in test harness without full engine init
     *
     * Produces correct output. ~30x slower than AVX2.
     * Kept as reference implementation and safety net.
     */
    const uint32_t ch = 3;
    const uint32_t src_w = src->width;
    const uint32_t src_h = src->height;

    const uint32_t x_ratio = (new_w > 1)
                                 ? (uint32_t)(((uint64_t)(src_w - 1) << 16) / (new_w - 1))
                                 : 0;
    const uint32_t y_ratio = (new_h > 1)
                                 ? (uint32_t)(((uint64_t)(src_h - 1) << 16) / (new_h - 1))
                                 : 0;

    for (uint32_t dy = 0; dy < new_h; dy++)
    {
        uint32_t y_fp = dy * y_ratio;
        uint32_t y0 = y_fp >> 16;
        uint32_t y1 = (y0 + 1 < src_h) ? y0 + 1 : y0;
        uint32_t fy = y_fp & 0xffffu;
        uint32_t wy0 = 65536u - fy;
        const uint8_t *row0 = src->data + (size_t)y0 * src->stride;
        const uint8_t *row1 = src->data + (size_t)y1 * src->stride;
        uint8_t *dst_row = mem + (size_t)dy * stride;

        for (uint32_t dx = 0; dx < new_w; dx++)
        {
            uint32_t x_fp = dx * x_ratio;
            uint32_t x0 = x_fp >> 16;
            uint32_t x1 = (x0 + 1 < src_w) ? x0 + 1 : x0;
            uint32_t fx = x_fp & 0xffffu;
            uint32_t wx0 = 65536u - fx;
            const uint8_t *p00 = row0 + (size_t)x0 * ch;
            const uint8_t *p10 = row0 + (size_t)x1 * ch;
            const uint8_t *p01 = row1 + (size_t)x0 * ch;
            const uint8_t *p11 = row1 + (size_t)x1 * ch;
            uint8_t *out = dst_row + (size_t)dx * ch;

            for (uint32_t c = 0; c < ch; c++)
            {
                uint64_t top = (uint64_t)p00[c] * wx0 + (uint64_t)p10[c] * fx;
                uint64_t bottom = (uint64_t)p01[c] * wx0 + (uint64_t)p11[c] * fx;
                uint64_t value = top * wy0 + bottom * fy;
                out[c] = (uint8_t)((value + (1ULL << 31)) >> 32);
            }
        }
    }
}

// // src/pipeline/layout_resize_resample_bilinear.c  (REPLACE)

// #define _GNU_SOURCE

// #include "pipeline/layout_resize_internal.h"
// #include "pipeline/jump_table.h"
// #include "arch/resize_params.h"
// #include "core/opcodes.h"
// #include "core/context_internal.h"

// void img_layout_resize_resample_bilinear(
//     const img_buffer_t *src,
//     uint8_t *mem,
//     uint32_t new_w,
//     uint32_t new_h,
//     uint32_t stride)
// {
//     /*
//      * If an AVX2/SIMD resize kernel is registered in the jump table,
//      * use it. This is O(1) lookup followed by a vectorized kernel
//      * that processes 32 bytes (10 pixels) per cycle instead of 1.
//      *
//      * The kernel writes into the already-allocated dst buffer (mem).
//      * We construct a temporary img_buffer_t pointing at mem so the
//      * kernel has the correct width/height/stride metadata.
//      */
//     img_kernel_fn resize_fn = g_jump_table[OP_RESIZE];

//     if (__builtin_expect(resize_fn != NULL, 1))
//     {
//         /* Build output descriptor — points at pre-allocated mem */
//         img_buffer_t dst_buf = {
//             .data = mem,
//             .width = new_w,
//             .height = new_h,
//             .channels = 3,
//             .stride = stride,
//         };

//         /* Build resize params — kernel reads src, writes dst */
//         resize_params_t params = {
//             .src = (struct img_buffer *)src,
//             .target_w = new_w,
//             .target_h = new_h,
//             .scale_x = 0, /* kernel computes internally */
//             .scale_y = 0,
//             .x_index = NULL,
//             .y_index = NULL,
//         };

//         /*
//          * Stack ctx — layout has no per-job ctx at this call site.
//          * The AVX2 resize kernel only reads ctx->local_pool and
//          * ctx->caps, both of which are safe to leave zero for
//          * a pure SIMD resize (no allocations inside the kernel).
//          */
//         img_ctx_t ctx = {0};

//         resize_fn(&ctx, &dst_buf, &params);
//         return;
//     }

//     /*
//      * Fallback: pure C scalar bilinear.
//      * Executes when jump table is not initialized (engine init
//      * hasn't run yet) or on architectures without SIMD.
//      * Correct output, ~10-20x slower than AVX2.
//      */
//     const uint32_t ch = 3;
//     const uint32_t src_w = src->width;
//     const uint32_t src_h = src->height;

//     const uint32_t x_ratio = (new_w > 1)
//                                  ? (uint32_t)(((uint64_t)(src_w - 1) << 16) / (new_w - 1))
//                                  : 0;
//     const uint32_t y_ratio = (new_h > 1)
//                                  ? (uint32_t)(((uint64_t)(src_h - 1) << 16) / (new_h - 1))
//                                  : 0;

//     for (uint32_t dy = 0; dy < new_h; dy++)
//     {
//         uint32_t y_fp = dy * y_ratio;
//         uint32_t y0 = y_fp >> 16;
//         uint32_t y1 = (y0 + 1 < src_h) ? y0 + 1 : y0;
//         uint32_t fy = y_fp & 0xffffu;
//         uint32_t wy0 = 65536u - fy;

//         const uint8_t *row0 = src->data + (size_t)y0 * src->stride;
//         const uint8_t *row1 = src->data + (size_t)y1 * src->stride;
//         uint8_t *dst_row = mem + (size_t)dy * stride;

//         for (uint32_t dx = 0; dx < new_w; dx++)
//         {
//             uint32_t x_fp = dx * x_ratio;
//             uint32_t x0 = x_fp >> 16;
//             uint32_t x1 = (x0 + 1 < src_w) ? x0 + 1 : x0;
//             uint32_t fx = x_fp & 0xffffu;
//             uint32_t wx0 = 65536u - fx;

//             const uint8_t *p00 = row0 + (size_t)x0 * ch;
//             const uint8_t *p10 = row0 + (size_t)x1 * ch;
//             const uint8_t *p01 = row1 + (size_t)x0 * ch;
//             const uint8_t *p11 = row1 + (size_t)x1 * ch;
//             uint8_t *out = dst_row + (size_t)dx * ch;

//             for (uint32_t c = 0; c < ch; c++)
//             {
//                 uint64_t top = (uint64_t)p00[c] * wx0 + (uint64_t)p10[c] * fx;
//                 uint64_t bottom = (uint64_t)p01[c] * wx0 + (uint64_t)p11[c] * fx;
//                 uint64_t value = top * wy0 + bottom * fy;
//                 out[c] = (uint8_t)((value + (1ULL << 31)) >> 32);
//             }
//         }
//     }
// }
