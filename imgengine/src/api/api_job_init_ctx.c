// ./src/api/api_job_init_ctx.c
#include "api/api_job_internal.h"

img_result_t img_api_init_ctx(img_engine_t *engine, img_ctx_t *ctx) {
    if (!engine || !ctx || !engine->global_pool)
        return IMG_ERR_SECURITY;

    img_api_make_ctx(engine, ctx);
    return IMG_SUCCESS;
}
