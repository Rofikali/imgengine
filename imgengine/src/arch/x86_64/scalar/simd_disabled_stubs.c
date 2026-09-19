#include "arch/arch_interface.h"
#include "pipeline/batch_exec.h"

void img_arch_avx2_resize(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    resize_scalar(ctx, dst, params);
}

void resize_avx2(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    resize_scalar(ctx, dst, params);
}

void img_arch_resize_avx512(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    resize_scalar(ctx, dst, params);
}

void resize_avx512(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    resize_scalar(ctx, dst, params);
}

void img_arch_resize_h_avx2(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    img_arch_resize_h_scalar(ctx, dst, params);
}

void img_arch_resize_v_avx2(img_ctx_t *ctx, img_buffer_t *dst, void *params) {
    img_arch_resize_v_scalar(ctx, dst, params);
}

void img_arch_grayscale_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    img_arch_grayscale_scalar(ctx, buf, params);
}

void img_batch_resize_fused_avx2(img_ctx_t *ctx, void *batch, void *params) {
    (void)ctx;
    (void)batch;
    (void)params;
}

void img_fused_resize_color_norm_avx2(img_ctx_t *ctx, img_batch_t *batch, void *params) {
    (void)ctx;
    (void)batch;
    (void)params;
}
