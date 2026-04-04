// io/encoder/stb_bridge.c

#define STB_IMAGE_IMPLEMENTATION

#include "api/v1/img_types.h"
#include "core/context_internal.h"
#include "api/v1/img_error.h"
#include <string.h>
#include "api/v1/img_buffer_utils.h"

img_result_t img_decode_stb(
    img_ctx_t *ctx,
    const uint8_t *data,
    size_t size,
    img_buffer_t *out)
{
    int w, h, ch;

    uint8_t *decoded = stbi_load_from_memory(
        data, size, &w, &h, &ch, 3);

    if (!decoded)
        return IMG_ERR_FORMAT;

    uint8_t *mem = img_slab_alloc(ctx->local_pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    memcpy(mem, decoded, w * h * 3);

    *out = img_buffer_create(mem, w, h, 3);

    stbi_image_free(decoded);
    return IMG_SUCCESS;
}