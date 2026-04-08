// src/pipeline/fused_dispatch.c

#include "pipeline/pipeline_signature.h"
#include "pipeline/fused_kernel.h"

/* forward declarations */
void kernel_none(img_ctx_t *, img_buffer_t *);
void kernel_gray(img_ctx_t *, img_buffer_t *);
void kernel_bright(img_ctx_t *, img_buffer_t *);
void kernel_gray_bright(img_ctx_t *, img_buffer_t *);
void kernel_gray_resize(img_ctx_t *, img_buffer_t *);
void kernel_all(img_ctx_t *, img_buffer_t *);

static img_fused_kernel_fn g_dispatch[8] = {
    [0] = kernel_none,
    [1] = kernel_gray,
    [2] = kernel_bright,
    [3] = kernel_gray_bright,
    [5] = kernel_gray_resize,
    [7] = kernel_all,
};

img_fused_kernel_fn img_get_fused_kernel(uint32_t sig)
{
    return g_dispatch[sig];
}
// extern void kernel_none(img_ctx_t*, img_buffer_t*, img_fused_params_t*);
// extern void kernel_gray(img_ctx_t*, img_buffer_t*, img_fused_params_t*);
// extern void kernel_bright(img_ctx_t*, img_buffer_t*, img_fused_params_t*);
// extern void kernel_gray_bright(img_ctx_t*, img_buffer_t*, img_fused_params_t*);
// extern void kernel_gray_resize(img_ctx_t*, img_buffer_t*, img_fused_params_t*);
// extern void kernel_all(img_ctx_t*, img_buffer_t*, img_fused_params_t*);