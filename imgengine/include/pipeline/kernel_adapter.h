// include/pipeline/kernel_adapter.h

#ifndef IMGENGINE_KERNEL_ADAPTER_H
#define IMGENGINE_KERNEL_ADAPTER_H

#include "api/v1/img_plugin_api.h"

/*
 * 🔥 INTERNAL KERNEL ABI
 */
typedef void (*img_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *,
    void *);

/*
 * 🔥 ADAPTER (ZERO OVERHEAD INLINE)
 */
static inline img_kernel_fn img_adapt_op(img_op_fn fn)
{
    return (img_kernel_fn)fn;
}

/*
 * 🔥 SAFE WRAPPER (if you want strict mode)
 */
static inline void img_kernel_wrapper(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    void *params)
{
    (void)params;
    ((img_op_fn)params)(ctx, buf);
}

#endif