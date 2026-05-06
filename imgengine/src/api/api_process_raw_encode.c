// ./src/api/api_process_raw_encode.c
#include "api/api_process_raw_internal.h"
#include "api/io_vtable.h"

img_result_t img_api_process_raw_encode(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **output,
                                        size_t *output_size) {
    if (!g_io_vtable.encode)
        return IMG_ERR_INTERNAL;

    return (img_result_t)g_io_vtable.encode(ctx, buf, output, output_size);
}
