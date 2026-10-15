// ./src/api/api_process_raw.c
#include "api/v1/img_api.h"
#include "api/api_process_raw_internal.h"

img_result_t img_api_process_raw(img_engine_t *engine, uint8_t *input, size_t input_size,
                                 uint8_t **output, size_t *output_size) {
    img_ctx_t ctx = {0};
    img_buffer_t buf = {0};

    img_result_t sec =
        img_api_process_raw_prepare(engine, input, output, output_size, input_size, &ctx);
    if (sec != IMG_SUCCESS)
        return sec;

    img_result_t exec =
        img_api_process_raw_execute(engine, &ctx, input, input_size, &buf, output, output_size);
    return exec;
}
