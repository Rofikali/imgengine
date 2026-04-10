
// ./src/io/encoder/encoder_entry.c

#include "io/encoder/encoder_entry.h"
#include <turbojpeg.h>
#include <stdlib.h>
// #include "api/v1/img_types.h"
#include "core/buffer.h"

int img_encode_from_buffer(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    uint8_t **out_data,
    size_t *out_size)
{
    if (!ctx || !buf || !out_data || !out_size)
        return -1;

    tjhandle tj = tjInitCompress();
    if (!tj)
        return -1;

    unsigned char *jpegBuf = NULL;
    unsigned long jpegSize = 0;

    if (tjCompress2(
            tj,
            buf->data,
            buf->width,
            buf->stride,
            buf->height,
            TJPF_RGB,
            &jpegBuf,
            &jpegSize,
            TJSAMP_444,
            85,
            TJFLAG_FASTDCT) != 0)
    {
        tjDestroy(tj);
        return -1;
    }

    // 🔥 TRANSFER OWNERSHIP TO CALLER
    *out_data = jpegBuf;
    *out_size = jpegSize;

    tjDestroy(tj);
    return 0;
}