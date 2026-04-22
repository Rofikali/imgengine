
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
        g_jump_table[OP_RESIZE] = img_arch_avx2_resize;
        // 🔥 fallback to AVX2 for now
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
    }

    // ================= AVX2 =================
    else if (img_cpu_has_avx2(caps))
    {
        g_jump_table[OP_RESIZE] = img_arch_avx2_resize;
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
    }

    // ================= NEON =================
    else if (img_cpu_has_neon(caps))
    {
        g_jump_table[OP_RESIZE] = resize_scalar;
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
    }

    // ================= SCALAR =================
    else
    {
        g_jump_table[OP_RESIZE] = resize_scalar;
        g_jump_table[OP_RESIZE_H] = img_arch_resize_h_scalar;
        g_jump_table[OP_RESIZE_V] = img_arch_resize_v_scalar;
        g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
    }
}

// ================================================================
// FIX 1: src/pipeline/hardware_registry.c
//
// Add OP_RESIZE to every branch.
// OP_RESIZE      = single-pass combined resize (used by layout.c)
// OP_RESIZE_H/V  = two-pass separable (used by fused batch path)
// Both must be registered. Currently only H/V are registered.
// ================================================================

// #include "pipeline/hardware_registry.h"
// #include "pipeline/jump_table.h"
// #include "arch/arch_interface.h"
// #include "core/opcodes.h"

// void img_hw_register_kernels(cpu_caps_t caps)
// {
//     // ── AVX-512 ──────────────────────────────────────────────
//     if (img_cpu_has_avx512(caps))
//     {
//         /*
//          * AVX-512 resize: use AVX2 kernels until avx512.c implements
//          * the full OP_RESIZE path. When avx512 is ready, swap these.
//          */
//         // g_jump_table[OP_RESIZE] = img_arch_avx2_resize; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_h_avx2; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_v_avx2; /* FIX: was missing */
//         g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
//         g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
//     }

//     // ── AVX2 ─────────────────────────────────────────────────
//     else if (img_cpu_has_avx2(caps))
//     {
//         // g_jump_table[OP_RESIZE] = img_arch_avx2_resize; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_h_avx2; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_v_avx2; /* FIX: was missing */
//         g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2;
//         g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
//     }

//     // ── NEON (ARM) ────────────────────────────────────────────
//     else if (img_cpu_has_neon(caps))
//     {
//         /*
//          * NEON: no NEON resize implemented yet.
//          * Fall back to scalar for OP_RESIZE.
//          * NEON-specific kernels can be wired here when implemented.
//          */
//         // g_jump_table[OP_RESIZE] = img_arch_resize_scalar;   /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_v_scalar; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_h_scalar; /* FIX: was missing */
//         g_jump_table[OP_RESIZE_H] = img_arch_resize_h_avx2; /* wrong on NEON — fix later */
//         g_jump_table[OP_RESIZE_V] = img_arch_resize_v_avx2; /* wrong on NEON — fix later */
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
//     }

//     // ── Scalar fallback ───────────────────────────────────────
//     else
//     {
//         g_jump_table[OP_RESIZE] = img_arch_resize_v_scalar; /* FIX: was missing */
//         g_jump_table[OP_RESIZE] = img_arch_resize_h_scalar; /* FIX: was missing */
//         g_jump_table[OP_RESIZE_H] = img_arch_resize_h_scalar;
//         g_jump_table[OP_RESIZE_V] = img_arch_resize_v_scalar;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_avx2;
//         g_jump_table[OP_GRAYSCALE] = img_arch_grayscale_scalar;
//     }
// }
