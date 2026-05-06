// ./src/api/api_job_finish_output_file.c
#include "api/api_job_finish_output_internal.h"
#include "api/io_vtable.h"

#include <stdio.h>

img_result_t img_api_finish_job_output_file(img_ctx_t *ctx, img_canvas_t *canvas,
                                            const img_layout_t *layout, const img_job_t *job,
                                            const char *output_path) {
    uint8_t *out = NULL;
    size_t out_size = 0;

    img_result_t r = img_api_render_job_output(ctx, canvas, layout, job, &out, &out_size);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "[JOB] encode buffer failed: %s\n", img_result_name(r));
        img_encoded_free(out);
        return r;
    }

    if (!out)
        return IMG_ERR_IO;

    r = img_write_buffer_file(output_path, out, out_size);
    img_encoded_free(out);
    if (r != IMG_SUCCESS)
        return r;

    return IMG_SUCCESS;
}
