// ./src/api/api_process_raw_execute.c
#include "api/api_process_raw_internal.h"

img_result_t img_api_process_raw_execute(img_engine_t *engine, img_ctx_t *ctx, const uint8_t *input,
                                         size_t input_size, img_buffer_t *buf, uint8_t **output,
                                         size_t *output_size) {
    img_result_t dec = img_api_process_raw_decode(ctx, input, input_size, buf);
    if (dec != IMG_SUCCESS) {
        img_api_process_raw_free_buffer(engine, buf);
        return dec;
    }

    img_result_t enc = img_api_process_raw_encode(ctx, buf, output, output_size);
    img_api_process_raw_free_buffer(engine, buf);
    return enc;
}
