// src/pipeline/fused_registry.c

#include "pipeline/batch_exec.h"
#include "arch/cpu_caps.h"

extern void img_fused_resize_color_norm_avx2(
    img_ctx_t *, img_batch_t *, void *);

static img_fused_kernel_fn g_fused_kernel = 0;

void img_fused_init(cpu_caps_t caps)
{
    if (img_cpu_has_avx2(caps))
    {
        g_fused_kernel = img_fused_resize_color_norm_avx2;
    }
}

img_fused_kernel_fn img_get_fused_kernel(void)
{
    return g_fused_kernel;
}