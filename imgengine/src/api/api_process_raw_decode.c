// ./src/api/api_process_raw_decode.c
#include "api/api_process_raw_internal.h"
#include "api/io_vtable.h"

img_result_t img_api_process_raw_decode(
    img_ctx_t *ctx,
    const uint8_t *input,
    size_t input_size,
    img_buffer_t *buf)
{
    if (!g_io_vtable.decode)
        return IMG_ERR_INTERNAL;

    return (img_result_t)g_io_vtable.decode(ctx, input, input_size, buf);
}
