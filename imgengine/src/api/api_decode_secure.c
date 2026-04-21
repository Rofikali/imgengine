// ./src/api/api_decode_secure.c

#include "api/api_internal.h"
#include "io/io_vtable.h"

img_result_t decode_image_secure(
    img_engine_t *engine,
    const uint8_t *input,
    size_t size,
    img_buffer_t *out_buf)
{
    if (!engine || !input || !out_buf)
        return IMG_ERR_SECURITY;
    if (!g_io_vtable.decode)
        return IMG_ERR_INTERNAL;

    img_ctx_t ctx = {0};
    img_api_make_ctx(engine, &ctx);

    int rc = g_io_vtable.decode(&ctx, input, size, out_buf);
    return (img_result_t)rc;
}
