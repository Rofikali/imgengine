// io/encoder/encoder_entry.c

#include "io/encoder/encoder_entry.h"
#include <turbojpeg.h>
#include <stdlib.h>
// #include "api/v1/img_types.h"

int img_encode_from_buffer(img_ctx_t *ctx,
                           img_buffer_t *buf,
                           uint8_t **out_data,
                           size_t *out_size)
{
    if (!ctx || !buf || !out_data || !out_size)
        return -1;

    tjhandle tj = tjInitCompress();
    // tjhandle tj = ctx->tj_encoder;

    if (!tj)
        return -1;

    unsigned char *jpegBuf = NULL; // TurboJPEG will allocate
    unsigned long jpegSize = 0;

    int width = buf->width;
    int height = buf->height;
    int pitch = buf->stride;

    int quality = 85; // 🔥 you can make this dynamic later

    // 🔥 ZERO-COPY INPUT (no memcpy here)
    if (tjCompress2(tj,
                    buf->data, // ← directly from SLAB
                    width,
                    pitch,
                    height,
                    TJPF_RGB,
                    &jpegBuf,
                    &jpegSize,
                    TJSAMP_444,
                    quality,
                    TJFLAG_FASTDCT) != 0)
    {
        tjDestroy(tj);
        // return -1;
    }

    // 🔥 TRANSFER OWNERSHIP (important)
    *out_data = jpegBuf;
    *out_size = jpegSize;

    tjDestroy(tj);
    return 0;
}
