// ./src/pipeline/pipeline_fuse_exec_avx2.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_exec_internal.h"
#include "pipeline/fused_params.h"

#include <stddef.h>

void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf) {
    /* See img_arch_grayscale_avx2: correctness precedes vectorization. */
    fused_exec_scalar(ctx, buf);
}
