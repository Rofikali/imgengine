// ./src/api/api_process_common.c
#include "api/api_internal.h"

void img_api_make_ctx(const img_engine_t *engine, img_ctx_t *ctx)
{
    img_ctx_bind_engine(engine, ctx);
}
