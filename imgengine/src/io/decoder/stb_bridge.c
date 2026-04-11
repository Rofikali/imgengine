// ./src/io/decoder/stb_bridge.c

// ./src/io/decoder/stb_bridge.c

#include "core/context_internal.h"
#include "api/v1/img_error.h"
#include "api/v1/img_buffer_utils.h"
#include "memory/slab.h"
#include "src/third_party/stb/stb_image.h"
#include <string.h>

img_result_t img_decode_stb(
    img_ctx_t *ctx,
    const uint8_t *data,
    size_t size,
    img_buffer_t *out)
{
    int w, h, ch;

    uint8_t *decoded =
        stbi_load_from_memory(data, (int)size, &w, &h, &ch, 3);

    if (!decoded)
        return IMG_ERR_FORMAT;

    size_t required = (size_t)w * h * 3;
    size_t block = img_slab_block_size(ctx->local_pool);

    if (required > block)
    {
        stbi_image_free(decoded);
        return IMG_ERR_NOMEM;
    }

    uint8_t *mem = img_slab_alloc(ctx->local_pool);
    if (!mem)
    {
        stbi_image_free(decoded);
        return IMG_ERR_NOMEM;
    }

    memcpy(mem, decoded, required);

    // 🔥 CORRECT: construct buffer (not allocator misuse)
    out->data = mem;
    out->width = (uint32_t)w;
    out->height = (uint32_t)h;
    out->channels = 3;
    out->stride = (uint32_t)(w * 3);

    stbi_image_free(decoded);
    return IMG_SUCCESS;
}
