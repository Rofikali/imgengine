// ./src/api/api_job_render_output.c
#include "api/api_job_internal.h"
#include "api/io_vtable.h"
#include "core/context_internal.h"

img_result_t img_api_render_job_output(img_ctx_t *ctx, img_canvas_t *canvas,
                                       const img_layout_t *layout, const img_job_t *job,
                                       uint8_t **out, size_t *out_size) {
    if (!ctx || !canvas || !layout || !job || !out || !out_size)
        return IMG_ERR_SECURITY;
    if (!g_io_vtable.encode)
        return IMG_ERR_INTERNAL;

    *out = NULL;
    *out_size = 0;

    img_result_t r = img_apply_job_postfx(canvas, layout, job);
    if (r != IMG_SUCCESS)
        return r;

    r = (img_result_t)g_io_vtable.encode(ctx, &canvas->buf, out, out_size);
    if (r != IMG_SUCCESS) {
        img_encoded_free(*out);
        *out = NULL;
        *out_size = 0;
        return r;
    }

    if (!*out)
        return IMG_ERR_IO;

    return IMG_SUCCESS;
}
