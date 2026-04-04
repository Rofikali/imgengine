// /* pipeline/jump_table.c */

#include "pipeline/jump_table.h"
#include "plugins/plugin_internal.h" // for OP_RESIZE etc
#include <string.h>
#include "core/opcodes.h" // 🔥 ADD THIS

// ================= GLOBAL TABLES =================

img_op_fn g_jump_table[IMG_MAX_OPS] = {0};
img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS] = {0};

// ================= REGISTER =================

void img_register_op(uint32_t opcode, img_op_fn fn, img_batch_op_fn batch_fn)
{
    if (opcode >= IMG_MAX_OPS)
        return;

    g_jump_table[opcode] = fn;
    g_batch_jump_table[opcode] = batch_fn;
}

// ================= ARCH KERNEL DECL =================

// Scalar
void resize_scalar(img_ctx_t *, img_buffer_t *, void *);

// AVX2
void resize_avx2(img_ctx_t *, img_buffer_t *, void *);

// AVX512
void resize_avx512(img_ctx_t *, img_buffer_t *, void *);

// ADD THIS

extern void img_batch_resize_fused_avx2(
    img_ctx_t *, void *, void *);

// ================= INIT =================

void img_jump_table_init(cpu_caps_t caps)
{
    // 🔥 ALWAYS RESET (deterministic)
    memset(g_jump_table, 0, sizeof(g_jump_table));
    memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

    // memset(g_jump_table, 0, sizeof(g_jump_table));
    // memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));

    // ================= RESIZE =================
    if (img_cpu_has_avx512(caps))
    {
        img_register_op(OP_RESIZE, resize_avx512, NULL);
    }
    else if (img_cpu_has_avx2(caps))
    {
        img_register_op(OP_RESIZE, resize_avx2, NULL);
    }
    else if
    {
        img_register_op(OP_RESIZE, resize_scalar, NULL);
    }

    else
        (img_cpu_has_avx2(caps))
        {
            img_register_op(OP_RESIZE,
                            img_arch_resize_h_avx2,
                            img_batch_resize_fused_avx2);
        }

    // ================= FUTURE OPS =================
    // img_register_op(OP_GRAYSCALE, grayscale_avx2, NULL);
}

extern void img_kernel_fused_resize_color_norm_avx2(
    img_ctx_t *, img_buffer_t *, void *);

img_register_op(OP_RESIZE,
                img_kernel_fused_resize_color_norm_avx2,
                NULL);