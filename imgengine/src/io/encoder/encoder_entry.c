// ./src/io/encoder/encoder_entry.c

#include "io/encoder/encoder_entry.h"
#include <pthread.h>
#include <turbojpeg.h>
#include <stdlib.h>

#include "core/buffer.h"

static pthread_key_t g_tj_encoder_key;
static pthread_once_t g_tj_encoder_key_once = PTHREAD_ONCE_INIT;
static int g_tj_encoder_key_status;

static void img_destroy_thread_encoder(void *handle) {
    if (handle)
        tjDestroy((tjhandle)handle);
}

static void img_create_thread_encoder_key(void) {
    g_tj_encoder_key_status = pthread_key_create(&g_tj_encoder_key, img_destroy_thread_encoder);
}

static tjhandle img_get_thread_encoder(void) {
    if (pthread_once(&g_tj_encoder_key_once, img_create_thread_encoder_key) != 0 ||
        g_tj_encoder_key_status != 0)
        return NULL;

    tjhandle encoder = pthread_getspecific(g_tj_encoder_key);
    if (!encoder) {
        encoder = tjInitCompress();
        if (!encoder)
            return NULL;
        if (pthread_setspecific(g_tj_encoder_key, encoder) != 0) {
            tjDestroy(encoder);
            return NULL;
        }
    }

    return encoder;
}

void img_encoder_release_thread(void) {
    if (pthread_once(&g_tj_encoder_key_once, img_create_thread_encoder_key) != 0 ||
        g_tj_encoder_key_status != 0)
        return;

    tjhandle encoder = pthread_getspecific(g_tj_encoder_key);
    if (!encoder)
        return;

    (void)pthread_setspecific(g_tj_encoder_key, NULL);
    tjDestroy(encoder);
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
