// ./include/pipeline/kernel_adapter.h

// FILE: include/pipeline/kernel_adapter.h  (REPLACE)

#ifndef IMGENGINE_KERNEL_ADAPTER_H
#define IMGENGINE_KERNEL_ADAPTER_H

#include "pipeline/plugin_abi.h"

/*
 * KERNEL ABI — unified 3-arg signature
 *
 * All kernels (plugins, SIMD, scalar) use this signature.
 * params: op-specific parameter block (may be NULL for ops
 *         that read params from ctx->op_params).
 *
 * This eliminates the cast between img_op_fn and img_kernel_fn.
 * img_op_fn in img_plugin_api.h is redefined to match.
 */
typedef void (*img_kernel_fn)(img_ctx_t *ctx, img_buffer_t *buf, void *params);

typedef void (*img_batch_kernel_fn)(img_ctx_t *ctx, img_batch_t *batch, void *params);

/*
 * img_adapt_op() — removed.
 *
 * Previously cast img_op_fn (2-arg) to img_kernel_fn (3-arg).
 * Now both are the same type — no cast needed, no adapter needed.
 *
 * Plugin functions that ignore params just cast params to void:
 *   (void)params;
 */

#endif /* IMGENGINE_KERNEL_ADAPTER_H */

// // ./include/pipeline/kernel_adapter.h

// #ifndef IMGENGINE_KERNEL_ADAPTER_H
// #define IMGENGINE_KERNEL_ADAPTER_H

// #include "api/v1/img_plugin_api.h"

// /*
//  * 🔥 INTERNAL KERNEL ABI (ONLY USED IN HOT PATH)
//  */
// typedef void (*img_kernel_fn)(
//     img_ctx_t *,
//     img_buffer_t *,
//     void *);

// /*
//  * 🔥 ADAPT PLUGIN → KERNEL (ZERO COST)
//  */
// static inline img_kernel_fn img_adapt_op(img_op_fn fn)
// {
//     return (img_kernel_fn)fn;
// }

// /*
//  * 🔥 ADAPT BATCH (OPTIONAL FUTURE)
//  */
// typedef void (*img_batch_kernel_fn)(
//     img_ctx_t *,
//     img_batch_t *,
//     void *);

// #endif
