// ./src/api/api_process_raw_prepare.c
#include "api/api_process_raw_internal.h"
#include "api/api_internal.h"

img_result_t img_api_process_raw_prepare(const img_engine_t *engine, const uint8_t *input,
                                         uint8_t **output, size_t *output_size, size_t input_size,
                                         img_ctx_t *ctx) {
    img_result_t sec = img_api_process_raw_validate(engine, input, output, output_size, input_size);
    if (sec != IMG_SUCCESS)
        return sec;

    img_api_make_ctx(engine, ctx);
    return IMG_SUCCESS;
}
