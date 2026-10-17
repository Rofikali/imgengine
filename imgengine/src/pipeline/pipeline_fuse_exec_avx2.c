#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_exec_internal.h"

void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf) {
    /* Preserve dispatch while prioritising scalar-equivalent RGB semantics. */
    fused_exec_scalar(ctx, buf);
}
