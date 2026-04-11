// ./src/pipeline/hardware_registry.c

// ./src/pipeline/hardware_registry.c


#include "pipeline/hardware_registry.h"
#include "pipeline/jump_table.h"
#include "arch/arch_interface.h"
#include "core/opcodes.h"

void img_hw_register_kernels(cpu_caps_t caps)
{
    // ================= AVX512 =================
    if (img_cpu_has_avx512(caps))
    {
        // 🔥 fallback to AVX2 for now
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
    }

    // ================= AVX2 =================
    else if (img_cpu_has_avx2(caps))
    {
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
    }

    // ================= NEON =================
    else if (img_cpu_has_neon(caps))
    {
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
    }

    // ================= SCALAR =================
    else
    {
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_scalar;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_scalar;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
    }
}
