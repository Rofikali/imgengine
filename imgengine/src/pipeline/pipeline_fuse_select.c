// ./src/pipeline/pipeline_fuse_select.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_internal.h"
#include "pipeline/pipeline_fuse_exec_internal.h"

img_single_kernel_fn img_pipeline_fuse_select_single_fn(void) {
#if defined(__AVX2__)
    return fused_exec_avx2;
#else
    return fused_exec_scalar;
#endif
}
