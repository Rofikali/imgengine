// src/pipeline/jump_table.c

#include "pipeline/jump_table.h"
#include "core/opcodes.h"
#include <string.h>

/*
 * GLOBAL TABLES — SINGLE SOURCE OF TRUTH
 *
 * Defined here. Declared extern in jump_table.h.
 * Every translation unit that includes jump_table.h
 * sees the extern declaration and links to these definitions.
 *
 * g_jump_table:       single-image kernel dispatch (img_kernel_fn ABI)
 * g_batch_jump_table: batch kernel dispatch         (img_batch_kernel_fn ABI)
 *
 * Both are zero-initialized at startup, then populated by:
 *   img_jump_table_init()    — CPU-optimal kernels
 *   img_hw_register_kernels() — SIMD kernels
 *   img_plugins_init_all()   — plugin kernels (may override)
 */
img_kernel_fn g_jump_table[IMG_MAX_OPS];
img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS];

/*
 * INTERNAL ADAPTER (zero cost, inline)
 * Casts img_op_fn to img_kernel_fn — ABI-compatible by construction.
 */
static inline img_kernel_fn adapt(img_op_fn fn)
{
    return (img_kernel_fn)fn;
}

/*
 * img_register_op()
 *
 * Registers a plugin's single and/or batch kernel into the jump tables.
 * Called from img_plugins_init_all() and img_plugin_load_all().
 * Plugins may call this to override hardware kernels — by design.
 */
void img_register_op(
    uint32_t opcode,
    img_op_fn single_fn,
    img_batch_op_fn batch_fn)
{
    if (opcode >= IMG_MAX_OPS)
        return;

    if (single_fn)
        g_jump_table[opcode] = img_adapt_op(single_fn);

    if (batch_fn)
        g_batch_jump_table[opcode] = (img_batch_kernel_fn)batch_fn;
}

/*
 * ARCH KERNEL DECLARATIONS
 * Defined in src/arch/x86_64/ — resolved at link time.
 */
extern void resize_scalar(img_ctx_t *, img_buffer_t *, void *);
extern void resize_avx2(img_ctx_t *, img_buffer_t *, void *);
extern void resize_avx512(img_ctx_t *, img_buffer_t *, void *);
extern void img_batch_resize_fused_avx2(img_ctx_t *, img_batch_t *, void *);

/*
 * img_jump_table_init()
 *
 * Cold path — called once at engine startup.
 * Selects the best available kernel for each op based on CPU caps.
 * Plugins may override entries after this call.
 *
 * Zero-initializes both tables first — any op not explicitly set
 * will have a NULL entry. Callers must check for NULL before dispatch.
 */
void img_jump_table_init(cpu_caps_t caps)
{
    memset(g_jump_table, 0, sizeof(g_jump_table));
    memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

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

    /* plugins can override AFTER this — intentional */
}

// // ./src/pipeline/jump_table.c

// #include "pipeline/jump_table.h"
// #include "core/opcodes.h"
// #include <string.h>

// /*
//  * 🔥 GLOBAL TABLES (SINGLE SOURCE OF TRUTH)
//  */
// // img_kernel_fn g_jump_table[IMG_MAX_OPS];
// // img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS];

// /*
//  * 🔥 INTERNAL ADAPTER (ZERO COST)
//  */
// static inline img_kernel_fn adapt(img_op_fn fn)
// {
//     return (img_kernel_fn)fn;
// }

// /*
//  * 🔥 REGISTER (PLUGIN → KERNEL ABI)
//  */
// void img_register_op(
//     uint32_t opcode,
//     img_op_fn single_fn,
//     img_batch_op_fn batch_fn)
// {
//     if (opcode >= IMG_MAX_OPS)
//         return;

//     if (single_fn)
//         g_jump_table[opcode] = img_adapt_op(single_fn);

//     if (batch_fn)
//         g_batch_jump_table[opcode] = (img_batch_kernel_fn)batch_fn;
// }

// /*
//  * ================================
//  * 🔥 ARCH KERNEL DECLARATIONS
//  * ================================
//  */

// // Scalar
// extern void resize_scalar(img_ctx_t *, img_buffer_t *, void *);

// // AVX2
// extern void resize_avx2(img_ctx_t *, img_buffer_t *, void *);

// // AVX512
// extern void resize_avx512(img_ctx_t *, img_buffer_t *, void *);

// // Batch fused
// extern void img_batch_resize_fused_avx2(
//     img_ctx_t *, img_batch_t *, void *);

// /*
//  * ================================
//  * 🔥 INIT (CPU DISPATCH)
//  * ================================
//  */
// void img_jump_table_init(cpu_caps_t caps)
// {
//     (void)caps;

//     memset(g_jump_table, 0, sizeof(g_jump_table));
//     memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));
//     /*
//      * ================= RESIZE =================
//      */
//     if (img_cpu_has_avx512(caps))
//     {
//         g_jump_table[OP_RESIZE] = resize_avx512;
//     }
//     else if (img_cpu_has_avx2(caps))
//     {
//         g_jump_table[OP_RESIZE] = resize_avx2;
//         g_batch_jump_table[OP_RESIZE] = img_batch_resize_fused_avx2;
//     }
//     else
//     {
//         g_jump_table[OP_RESIZE] = resize_scalar;
//     }

//     /*
//      * 👉 plugins can override AFTER this if needed
//      */
// }
