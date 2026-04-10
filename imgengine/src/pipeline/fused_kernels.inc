// pipeline/fused_kernels.inc
//
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

#define DEFINE_KERNEL(NAME, DO_GRAY, DO_BRIGHT, DO_RESIZE)              \
void NAME(img_ctx_t *ctx, img_batch_t *batch, void *params)             \
{                                                                        \
    (void)ctx;                                                           \
    if (__builtin_expect(!batch || !params, 0))                          \
        return;                                                          \
    const img_fused_params_t *p = (const img_fused_params_t *)params;   \
    for (uint32_t b = 0; b < batch->count; b++) {                       \
        img_buffer_t *buf = batch->buffers[b];                           \
        if (__builtin_expect(!buf, 0))                                   \
            continue;                                                    \
        uint8_t *data   = buf->data;                                     \
        uint32_t n      = buf->width * buf->height * buf->channels;      \
        for (uint32_t i = 0; i < n; i++) {                              \
            uint8_t px = data[i];                                        \
            if (DO_GRAY) {                                               \
                px = (uint8_t)((px * 77u) >> 8);                        \
            }                                                            \
            if (DO_BRIGHT) {                                             \
                uint32_t tmp = (uint32_t)px + p->brightness_value;      \
                px = (tmp > 255u) ? 255u : (uint8_t)tmp;               \
            }                                                            \
            data[i] = px;                                                \
        }                                                                \
        if (DO_RESIZE) {                                                 \
            buf->width  = p->new_w;                                      \
            buf->height = p->new_h;                                      \
        }                                                                \
    }                                                                    \
}