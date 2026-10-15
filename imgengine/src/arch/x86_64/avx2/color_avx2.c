#include "arch/arch_interface.h"

void img_arch_grayscale_avx2(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    /*
     * RGB luma is pixel-aware. The former byte-wise AVX2 loop was not
     * equivalent to the scalar RGB-luma contract. Keep dispatch stable while
     * using the proven scalar kernel until a separately verified RGB
     * deinterleaving AVX2 implementation is available.
     */
    img_arch_grayscale_scalar(ctx, buf, params);
}
