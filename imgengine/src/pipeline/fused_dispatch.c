// ./src/pipeline/fused_dispatch.c

// src/pipeline/fused_dispatch.c

#include "pipeline/pipeline_signature.h"
#include "pipeline/fused_kernel.h"
#include "pipeline/pipeline_fused.h"
#include <stddef.h>

/*
 * FUSED DISPATCH TABLE
 *
 * Direct-mapped: signature bitmask → kernel function pointer.
 * Index = bitmask of active ops (SIG_OP_* flags OR'd together).
 *
 * Size = 8 covers all combinations of 3 ops (2^3 = 8).
 * Sparse entries (unsupported combos) remain NULL → caller falls back.
 *
 * HOT PATH CONTRACT:
 *   - Table is read-only after init
 *   - No locks, no branches, O(1) lookup
 *   - All kernel_* functions are pure (no global state)
 */
static const img_fused_kernel_fn g_fused_dispatch[8] = {
    [0] = kernel_none,
    [SIG_OP_GRAYSCALE] = kernel_gray,
    [SIG_OP_BRIGHTNESS] = kernel_bright,
    [SIG_OP_GRAYSCALE | SIG_OP_BRIGHTNESS] = kernel_gray_bright,
    [SIG_OP_GRAYSCALE | SIG_OP_RESIZE] = kernel_gray_resize,
    [SIG_OP_BRIGHTNESS | SIG_OP_RESIZE] = kernel_bright, /* fallback: no fused impl yet */
    [SIG_OP_GRAYSCALE | SIG_OP_BRIGHTNESS | SIG_OP_RESIZE] = kernel_all,
    /* index 7 = all three: covered above */
};

/*
 * img_get_fused_kernel()
 *
 * Returns pre-compiled fused kernel for the given pipeline signature.
 * Returns NULL if no fused kernel exists for this combination —
 * caller must fall back to sequential jump-table dispatch.
 *
 * Called once per pipeline compile (cold path), result cached in
 * img_pipeline_fused_t.fn for hot path reuse.
 */
img_fused_kernel_fn
img_get_fused_kernel(img_pipeline_sig_t sig)
{
    /* bounds guard: sig must not exceed table size */
    if (__builtin_expect(sig >= 8, 0))
        return NULL;

    return g_fused_dispatch[sig];
}
