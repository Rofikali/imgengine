// ./src/pipeline/fused_registry.c

// src/pipeline/fused_registry.c

#include "pipeline/jump_table.h"
#include "pipeline/batch_exec.h"
#include "arch/cpu_caps.h"
#include "core/opcodes.h"

/*
 * AVX2 batch-fused kernel (declared in arch layer)
 * Defined in: src/arch/x86_64/avx2/fused_resize_color_norm_avx2.c
 */
extern void img_fused_resize_color_norm_avx2(img_ctx_t *, img_batch_t *, void *);

/*
 * img_fused_init()
 *
 * Registers CPU-optimal batch kernels into g_batch_jump_table.
 * Called once at engine startup (cold path).
 *
 * This is NOT img_get_fused_kernel() — that function lives in
 * fused_dispatch.c and operates on pipeline signatures.
 *
 * This function selects the best hardware implementation for
 * batch-level fused ops and wires them into the batch jump table.
 */
void img_fused_init(cpu_caps_t caps) {
    if (img_cpu_has_avx2(caps)) {
        /*
         * Register AVX2 fused batch kernel for OP_RESIZE.
         * This handles the common case: resize + color norm
         * in a single pass over the batch.
         */
        g_batch_jump_table[OP_RESIZE] = (img_batch_kernel_fn)img_fused_resize_color_norm_avx2;
    }

    /*
     * AVX-512 fused batch kernel: future work.
     * NEON fused batch kernel: future work.
     * Scalar batch: handled by sequential single-image path.
     */
}
