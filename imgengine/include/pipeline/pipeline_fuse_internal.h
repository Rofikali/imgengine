// ./include/pipeline/pipeline_fuse_internal.h
#ifndef IMGENGINE_PIPELINE_FUSE_INTERNAL_H
#define IMGENGINE_PIPELINE_FUSE_INTERNAL_H

#include "pipeline/pipeline_fused.h"

extern img_fused_params_t g_fused_params;
img_single_kernel_fn img_pipeline_fuse_select_single_fn(void);

#endif /* IMGENGINE_PIPELINE_FUSE_INTERNAL_H */
