// ./src/io/encoder/encoder_entry.c

#include "io/encoder/encoder_entry.h"
#include <turbojpeg.h>
#include <stdlib.h>

#include "core/buffer.h"

static _Thread_local tjhandle g_tj_encoder = NULL;

static tjhandle img_get_thread_encoder(void) {
    if (!g_tj_encoder)
        g_tj_encoder = tjInitCompress();

    return g_tj_encoder;
}

int img_encode_from_buffer_ex(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **out_data,
                              size_t *out_size, int quality, int subsamp) {
    if (!ctx || !buf || !out_data || !out_size)
        return -1;

    tjhandle tj = img_get_thread_encoder();
    if (!tj)
        return -1;

    unsigned char *jpegBuf = NULL;
    unsigned long jpegSize = 0;

    if (tjCompress2(tj, buf->data, buf->width, buf->stride, buf->height, TJPF_RGB, &jpegBuf,
                    &jpegSize, subsamp, quality, TJFLAG_FASTDCT) != 0) {
        return -1;
    }

    // 🔥 TRANSFER OWNERSHIP TO CALLER
    *out_data = jpegBuf;
    *out_size = jpegSize;

    return 0;
}

int img_encode_from_buffer(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **out_data,
                           size_t *out_size) {
    return img_encode_from_buffer_ex(ctx, buf, out_data, out_size, 85, TJSAMP_444);
}
