/* pipeline/jump_table.c */

#include "pipeline/jump_table.h"
#include "core/opcodes.h"
#include <string.h>

/*
 * 🔥 GLOBAL TABLES (SINGLE SOURCE OF TRUTH)
 */
img_kernel_fn g_jump_table[IMG_MAX_OPS];
img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS];

/*
 * 🔥 INTERNAL ADAPTER (ZERO COST)
 */
static inline img_kernel_fn adapt(img_op_fn fn)
{
    return (img_kernel_fn)fn;
}

/*
 * 🔥 REGISTER (CALLED AT INIT / PLUGIN LOAD)
 */
void img_register_op(
    uint32_t opcode,
    img_op_fn single_fn,
    img_batch_op_fn batch_fn)
{
    if (opcode >= IMG_MAX_OPS)
        return;

    if (single_fn)
        g_jump_table[opcode] = adapt(single_fn);

    if (batch_fn)
        g_batch_jump_table[opcode] = batch_fn;
}

/*
 * ================================
 * 🔥 ARCH KERNEL DECLARATIONS
 * ================================
 */

// Scalar
extern void resize_scalar(img_ctx_t *, img_buffer_t *, void *);

// AVX2
extern void resize_avx2(img_ctx_t *, img_buffer_t *, void *);

// AVX512
extern void resize_avx512(img_ctx_t *, img_buffer_t *, void *);

// Batch fused
extern void img_batch_resize_fused_avx2(
    img_ctx_t *, img_batch_t *, void *);

/*
 * ================================
 * 🔥 INIT (CPU DISPATCH)
 * ================================
 */
void img_jump_table_init(cpu_caps_t caps)
{
    /*
     * 🔥 HARD RESET (deterministic, no stale pointers)
     */
    memset(g_jump_table, 0, sizeof(g_jump_table));
    memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

    /*
     * ================= RESIZE =================
     */
    if (img_cpu_has_avx512(caps))
    {
        g_jump_table[OP_RESIZE] = resize_avx512;
    }
    else if (img_cpu_has_avx2(caps))
    {
        g_jump_table[OP_RESIZE] = resize_avx2;
        g_batch_jump_table[OP_RESIZE] = img_batch_resize_fused_avx2;
    }
    else
    {
        g_jump_table[OP_RESIZE] = resize_scalar;
    }

    /*
     * 👉 plugins can override AFTER this if needed
     */
}

// #include "pipeline/jump_table.h"
// #include "pipeline/pipeline_fused.h"
// #include "api/v1/img_plugin_api.h"
// #include <string.h>
// #include "core/opcodes.h" // 🔥 ADD THIS
// #include "arch/arch_interface.h"
// #include "pipeline/jump_table.h"

// img_kernel_fn g_jump_table[IMG_MAX_OPS] = {0};

// // ================= GLOBAL TABLES =================

// img_op_fn g_jump_table[IMG_MAX_OPS] = {0};
// img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS] = {0};

// // ================= REGISTER =================

// void img_register_op(uint32_t opcode, img_op_fn fn, img_batch_op_fn batch_fn)
// {
//     if (opcode >= IMG_MAX_OPS)
//         return;

//     g_jump_table[opcode] = fn;
//     g_batch_jump_table[opcode] = batch_fn;
// }

// // ================= ARCH KERNEL DECL =================

// // Scalar
// void resize_scalar(img_ctx_t *, img_buffer_t *, void *);

// // AVX2
// void resize_avx2(img_ctx_t *, img_buffer_t *, void *);

// // AVX512
// void resize_avx512(img_ctx_t *, img_buffer_t *, void *);

// // ADD THIS

// // extern void img_batch_resize_fused_avx2(
// //     img_ctx_t *, void *, void *);

// extern void img_batch_resize_fused_avx2(
//     img_ctx_t *, img_batch_t *, void *);

// // ================= INIT =================

// void img_jump_table_init(cpu_caps_t caps)
// {
//     // 🔥 ALWAYS RESET (deterministic)
//     memset(g_jump_table, 0, sizeof(g_jump_table));
//     memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

//     // memset(g_jump_table, 0, sizeof(g_jump_table));
//     // memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

//     // ================= RESIZE =================
//     if (img_cpu_has_avx512(caps))
//     {
//         img_register_op(OP_RESIZE, resize_avx512, NULL);
//     }
//     else if (img_cpu_has_avx2(caps))
//     {
//         img_register_op(OP_RESIZE, resize_avx2, img_batch_resize_fused_avx2);
//     }
//     else
//     {
//         img_register_op(OP_RESIZE, resize_scalar, NULL);
//     }
// }

// /*
//  * 🔥 REGISTER (ADAPT ONCE)
//  */
// void img_register_op(uint32_t opcode, img_op_fn fn)
// {
//     if (opcode >= IMG_MAX_OPS || !fn)
//         return;

//     /*
//      * 🔥 ZERO COST CAST (VALID ABI)
//      */
//     g_jump_table[opcode] = (img_kernel_fn)fn;
// }

// void img_jump_table_init(cpu_caps_t caps)
// {
//     (void)caps;
// }

// extern void img_kernel_fused_resize_color_norm_avx2(
//     img_ctx_t *, img_buffer_t *, void *);
