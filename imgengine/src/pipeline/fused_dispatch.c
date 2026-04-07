// src/pipeline/fused_dispatch.c

#include "pipeline/pipeline_signature.h"
#include "pipeline/fused_kernel.h"

/*
 * 🔥 DIRECT LOOKUP TABLE
 */
static img_fused_kernel_fn g_dispatch[8] = {
    [0] = kernel_none,
    [SIG_OP_GRAYSCALE] = kernel_gray,
    [SIG_OP_BRIGHTNESS] = kernel_bright,
    [SIG_OP_GRAYSCALE | SIG_OP_BRIGHTNESS] = kernel_gray_bright,
    [SIG_OP_GRAYSCALE | SIG_OP_RESIZE] = kernel_gray_resize,
    [SIG_OP_GRAYSCALE | SIG_OP_BRIGHTNESS | SIG_OP_RESIZE] = kernel_all,
};

img_fused_kernel_fn
img_get_fused_kernel(img_pipeline_sig_t sig)
{
    return g_dispatch[sig];
}