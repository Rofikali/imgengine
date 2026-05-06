// ./src/api/api_process_fast.c
#include "api/v1/img_api.h"
#include "api/api_process_fast_internal.h"

img_result_t img_api_process_fast(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                  img_pipeline_desc_t *pipe, img_buffer_t *out_buf) {
    img_result_t sec = img_api_process_fast_validate(engine, input, pipe, out_buf, input_size);

    if (sec != IMG_SUCCESS)
        return sec;

    img_result_t res = img_api_process_fast_decode(engine, input, input_size, out_buf);

    if (res != IMG_SUCCESS)
        return res;

    return img_api_process_fast_run(engine, pipe, out_buf);
}
