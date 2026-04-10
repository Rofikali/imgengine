
/* pipeline/engine.h */

#ifndef IMGENGINE_PIPELINE_ENGINE_H
#define IMGENGINE_PIPELINE_ENGINE_H

// Forward declarations (opaque)
typedef struct img_ctx img_ctx_t;
// typedef struct img_buffer img_buffer_t;
// typedef struct img_pipeline_desc img_pipeline_desc_t;

// void img_pipeline_execute(img_ctx_t *ctx, img_pipeline_desc_t *pipe, img_buffer_t *buf);
static inline void img_pipeline_execute(img_ctx_t *ctx, img_pipeline_compiled_t *p, void *data) {
    uint32_t n = p->count;

    img_kernel_fn *ops = p->ops;
    void **params = p->params;

#pragma GCC unroll 8
    for (uint32_t i = 0; i < n; i++) {
        ops[i](ctx, data, params[i]);
    }
}

#endif