// ./include/pipeline/pipeline_fuse_exec_internal.h
#ifndef IMGENGINE_PIPELINE_FUSE_EXEC_INTERNAL_H
#define IMGENGINE_PIPELINE_FUSE_EXEC_INTERNAL_H

#include "core/context_internal.h"
#include "core/buffer.h"

void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf);
void fused_exec_avx2(img_ctx_t *ctx, img_buffer_t *buf);

#endif /* IMGENGINE_PIPELINE_FUSE_EXEC_INTERNAL_H */
