// ./src/api/api_decode_secure.c

#include "api/api_internal.h"
#include "io/io_vtable.h"

#include <string.h>

img_result_t img_api_prepare_decoder(img_engine_t *engine, img_prepared_decoder_t *state) {
    if (!engine || !state)
        return IMG_ERR_SECURITY;

    memset(state, 0, sizeof(*state));
    state->engine = engine;
    img_api_make_ctx(engine, &state->ctx);
    return state->ctx.local_pool ? IMG_SUCCESS : IMG_ERR_NOMEM;
}

img_result_t img_api_decode_prepared(img_prepared_decoder_t *state, const uint8_t *input,
                                     size_t size, img_buffer_t *out_buf) {
    if (!state || !state->engine || !input || !out_buf)
        return IMG_ERR_SECURITY;
    if (!g_io_vtable.decode)
        return IMG_ERR_INTERNAL;

    int rc = g_io_vtable.decode(&state->ctx, input, size, out_buf);
    return (img_result_t)rc;
}

void img_api_decoder_destroy(img_prepared_decoder_t *state) {
    if (!state)
        return;

    memset(state, 0, sizeof(*state));
}

img_result_t decode_image_secure(img_engine_t *engine, const uint8_t *input, size_t size,
                                 img_buffer_t *out_buf) {
    if (!engine || !input || !out_buf)
        return IMG_ERR_SECURITY;
    if (!g_io_vtable.decode)
        return IMG_ERR_INTERNAL;

    img_ctx_t ctx = {0};
    img_api_make_ctx(engine, &ctx);

    int rc = g_io_vtable.decode(&ctx, input, size, out_buf);
    return (img_result_t)rc;
}
