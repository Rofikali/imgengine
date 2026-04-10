
// ./src/pipeline/fused_kernels.c

#include "pipeline/fused_params.h"
#include "pipeline/fused_kernel.h"

/*
 * 🔥 INSTANTIATE KERNELS
 */

#include "pipeline/fused_kernels.inc"

DEFINE_KERNEL(kernel_none, 0, 0, 0)
DEFINE_KERNEL(kernel_gray, 1, 0, 0)
DEFINE_KERNEL(kernel_bright, 0, 1, 0)
DEFINE_KERNEL(kernel_gray_bright, 1, 1, 0)
DEFINE_KERNEL(kernel_gray_resize, 1, 0, 1)
DEFINE_KERNEL(kernel_all, 1, 1, 1)