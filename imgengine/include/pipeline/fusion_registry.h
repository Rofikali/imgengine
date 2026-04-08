// pipeline/fusion_registry.h

#ifndef IMGENGINE_FUSION_REGISTRY_H
#define IMGENGINE_FUSION_REGISTRY_H

#include "pipeline/fused_kernel.h"

#include <stdint.h>

typedef void (*img_fused_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *);

typedef struct
{
    uint64_t signature; // encoded ops
    img_fused_kernel_fn fn;

} img_fusion_entry_t;

#endif