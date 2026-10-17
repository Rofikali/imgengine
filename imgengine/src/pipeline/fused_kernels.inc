// pipeline/fused_kernels.inc

// BATCH KERNEL TEMPLATE
//
// Generates all fused kernel variants via DEFINE_KERNEL macro.
// Each generated function follows img_fused_kernel_fn ABI:
//
//   void NAME(img_ctx_t *ctx, img_batch_t *batch, void *params)
//
// Template parameters (compile-time constants, not runtime branches):
//   DO_GRAY   — apply grayscale conversion
//   DO_BRIGHT — apply brightness adjustment
//   DO_RESIZE — apply resize (stub: updates dimensions only)
//
// Rules:
//   - No malloc, no locks, no printf in any generated function
//   - params is always img_fused_params_t* (cast from void*)
//   - Scalar implementation only — SIMD lives in arch/ layer
//   - Must produce identical output to fused_exec_avx2 in pipeline_fuse.c

#define DEFINE_KERNEL(NAME, DO_GRAY, DO_BRIGHT, DO_RESIZE)                                         \
    void NAME(img_ctx_t *ctx, img_batch_t *batch, void *params) {                                  \
        (void)ctx;                                                                                 \
        if (__builtin_expect(!batch || !params, 0))                                                \
            return;                                                                                \
        const img_fused_params_t *p = (const img_fused_params_t *)params;                          \
        for (uint32_t b = 0; b < batch->count; b++) {                                              \
            img_buffer_t *buf = batch->buffers[b];                                                 \
            if (__builtin_expect(!buf, 0))                                                         \
                continue;                                                                          \
            if (DO_GRAY)                                                                           \
                img_arch_grayscale_scalar(ctx, buf, NULL);                                         \
            if (DO_BRIGHT) {                                                                       \
                for (uint32_t y = 0; y < buf->height; y++) {                                       \
                    uint8_t *row = buf->data + (size_t)y * buf->stride;                            \
                    for (uint32_t x = 0; x < buf->width * buf->channels; x++) {                    \
                        uint32_t tmp = (uint32_t)row[x] + p->brightness_value;                     \
                        row[x] = (tmp > 255u) ? 255u : (uint8_t)tmp;                               \
                    }                                                                              \
                }                                                                                  \
            }                                                                                      \
            if (DO_RESIZE) {                                                                       \
                buf->width = p->new_w;                                                             \
                buf->height = p->new_h;                                                            \
            }                                                                                      \
        }                                                                                          \
    }
