#include "imgengine/image.h"
#include <string.h>

void img_blit_fast(img_t *dst, const img_t *src, int x, int y)
{
    int channels = 3;

    for (int row = 0; row < src->height; row++)
    {
        unsigned char *dst_ptr =
            dst->data + ((y + row) * dst->width + x) * channels;

        unsigned char *src_ptr =
            src->data + (row * src->width) * channels;

        // compiler auto-vectorizes this
        memcpy(dst_ptr, src_ptr, src->width * channels);
    }
}