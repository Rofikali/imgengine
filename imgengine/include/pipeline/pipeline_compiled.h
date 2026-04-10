#ifndef IMGENGINE_PIPELINE_COMPILED_H
#define IMGENGINE_PIPELINE_COMPILED_H

#include "pipeline/kernel_types.h"
#include <stdint.h>

#define IMG_MAX_PIPELINE_OPS 32

typedef struct __attribute__((aligned(64))) {
    uint32_t count;
    uint32_t _pad;

    /*
     * 🔥 STRUCT OF ARRAYS (BETTER PREFETCH)
     */
    img_kernel_fn ops[IMG_MAX_PIPELINE_OPS];
    void *params[IMG_MAX_PIPELINE_OPS];

} img_pipeline_compiled_t;

int img_pipeline_compile(const void *in, img_pipeline_compiled_t *out);

#endif