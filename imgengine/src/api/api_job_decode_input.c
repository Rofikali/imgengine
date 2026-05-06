// ./src/api/api_job_decode_input.c
#include "api/api_job_internal.h"
#include "api/io_vtable.h"

img_result_t img_api_decode_input(img_ctx_t *ctx, uint8_t **file_data, size_t file_size,
                                  img_buffer_t *photo) {
    if (!ctx || !file_data || !*file_data || !photo)
        return IMG_ERR_SECURITY;
    if (!g_io_vtable.decode)
        return IMG_ERR_INTERNAL;

    img_result_t r = (img_result_t)g_io_vtable.decode(ctx, *file_data, file_size, photo);
    img_api_release_input(*file_data, file_size);
    *file_data = NULL;

    return r;
}
