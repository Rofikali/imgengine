// src/pipeline/fused_registry.c

#include "hot/batch_exec.h"
#include "arch/cpu_caps.h"
#include "pipeline/fusion_registry.h"
#include <stddef.h>

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

img_fused_kernel_fn img_lookup_fused(uint64_t sig)
{
    extern img_fusion_entry_t g_fusion_table[];

    for (int i = 0; g_fusion_table[i].fn; i++)
    {
        if (g_fusion_table[i].signature == sig)
            return g_fusion_table[i].fn;
    }

    return NULL;
}