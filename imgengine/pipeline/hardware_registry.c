// pipeline/hardware_registry.c

#include "pipeline/jump_table.h"
#include "arch/arch_interface.h"
#include "plugins/plugin_internal.h"

void img_registry_init_hardware(cpu_arch_t arch)
{
    switch (arch)
    {
    case ARCH_AVX512:
    case ARCH_AVX2:

        // 🔥 Separable passes (SIMD)
        img_pipeline_register_op(OP_RESIZE_H, img_arch_resize_h_avx2, NULL);
        img_pipeline_register_op(OP_RESIZE_V, img_arch_resize_v_avx2, NULL);

        break;

    case ARCH_NEON:

        img_pipeline_register_op(OP_RESIZE_H, img_arch_resize_h_neon, NULL);
        img_pipeline_register_op(OP_RESIZE_V, img_arch_resize_v_neon, NULL);

        break;

    default:

        img_pipeline_register_op(OP_RESIZE_H, img_arch_resize_h_scalar, NULL);
        img_pipeline_register_op(OP_RESIZE_V, img_arch_resize_v_scalar, NULL);

        break;
    }
}
