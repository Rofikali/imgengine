// ./src/ops/border.c

#include "imgengine/image.h"
#include <string.h>

// Declare your AVX2 blit from lbit_avx2.c
void img_blit_avx2(img_t *dst, const img_t *src, int x, int y);

int img_add_border(const img_t *src, img_t *dst, mem_pool_t *mp, int border) {
    int nw = src->width + (2 * border);
    int nh = src->height + (2 * border);

    if (!img_create(dst, mp, nw, nh, 3))
        return 0;

    // 1. Fill entire new image with black (or border color) instantly
    memset(dst->data, 0, (size_t)nw * nh * 3);

    // 2. Use your AVX2 Blit to punch the source image into the center
    img_blit_avx2(dst, src, border, border);

    return 1;
}
