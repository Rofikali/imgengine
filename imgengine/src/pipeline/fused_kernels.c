// ./src/pipeline/fused_kernels.c

// FUSED KERNEL INSTANTIATION
//
// This file instantiates all fused kernel variants via DEFINE_KERNEL macro.
// Each kernel handles a specific combination of ops in a single batch pass.
//
// HOT PATH CONTRACT:
//   - No malloc/free
//   - No locks
//   - No printf
//   - All kernels are pure (no global state reads/writes)
//   - params pointer is always valid (checked by caller)
//
// ABI: img_fused_kernel_fn — (img_ctx_t*, img_batch_t*, void*)
//
// Scalar only. SIMD variants live in arch/x86_64/avx2/ and arch/aarch64/neon/

#include <stdint.h>
#include <stddef.h>

#include "core/buffer.h"           /* full img_buffer_t definition — REQUIRED */
#include "pipeline/batch.h"        /* img_batch_t definition */
#include "pipeline/fused_params.h" /* img_fused_params_t — brightness_value, new_w, new_h */
#include "core/context_internal.h" /* img_ctx_t — required by kernel ABI */

/*
 * DEFINE_KERNEL macro instantiation.
 * Include the template BEFORE using the macro.
 * The .inc file defines DEFINE_KERNEL — it does NOT instantiate anything.
 */
#include "src/pipeline/fused_kernels.inc"

/*
 * KERNEL INSTANTIATION TABLE
 *
 * Each line generates one complete kernel function.
 * Args: (name, do_gray, do_bright, do_resize)
 *
 * ABI-STABLE: names match declarations in include/pipeline/fused_kernel.h
 * DO NOT reorder or rename without updating:
 *   - fused_kernel.h (extern declarations)
 *   - fused_dispatch.c (dispatch table)
 */
DEFINE_KERNEL(kernel_none, 0, 0, 0)
DEFINE_KERNEL(kernel_gray, 1, 0, 0)
DEFINE_KERNEL(kernel_bright, 0, 1, 0)
DEFINE_KERNEL(kernel_gray_bright, 1, 1, 0)
DEFINE_KERNEL(kernel_gray_resize, 1, 0, 1)
DEFINE_KERNEL(kernel_all, 1, 1, 1)

/*
 * Include declarations AFTER definitions.
 * This satisfies the extern declarations in fused_kernel.h
 * without causing a conflicting-type error.
 *
 * Why after? The macro above generates the actual definitions.
 * If fused_kernel.h is included before the macro expands,
 * the compiler sees an extern declaration before the definition —
 * but if the signatures differ (they did before this fix), it errors.
 * Including after the definitions ensures the compiler validates
 * the extern against the actual generated signature.
 */
#include "pipeline/fused_kernel.h"