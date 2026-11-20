// ./src/io/decoder/stb_bridge.c

#include "core/context_internal.h"
#include "core/result.h"
#include "core/buffer.h"
#include "memory/slab.h"
#include "src/third_party/stb/stb_image.h"
#include "security/input_validator.h"
#include <limits.h>
#include <string.h>

static uint32_t img_read_u32_be(const uint8_t *input) {
    return ((uint32_t)input[0] << 24) | ((uint32_t)input[1] << 16) |
           ((uint32_t)input[2] << 8) | (uint32_t)input[3];
}

/* stb_image reserves IDAT storage before confirming the declared chunk is
 * present. Validate all PNG chunk bounds first so a hostile length cannot
 * induce allocation from data outside the supplied input. */
static img_result_t img_validate_png_chunks(const uint8_t *data, size_t size) {
    static const uint8_t signature[] = {0x89, 0x50, 0x4e, 0x47,
                                        0x0d, 0x0a, 0x1a, 0x0a};
    size_t offset = 0;
    int have_ihdr = 0;
    int have_idat = 0;

    if (size < sizeof(signature) || memcmp(data, signature, sizeof(signature)) != 0)
        return IMG_SUCCESS;

    offset = sizeof(signature);
    while (offset < size) {
        if (size - offset < 12u)
            return IMG_ERR_SECURITY;

        const uint32_t length = img_read_u32_be(data + offset);
        const uint8_t *const type = data + offset + 4u;
        offset += 8u;
        if (size - offset < 4u || (size_t)length > size - offset - 4u)
            return IMG_ERR_SECURITY;

        if (memcmp(type, "IHDR", 4) == 0) {
            if (have_ihdr || have_idat || length != 13u)
                return IMG_ERR_SECURITY;
            have_ihdr = 1;
        } else if (memcmp(type, "IDAT", 4) == 0) {
            if (!have_ihdr)
                return IMG_ERR_SECURITY;
            have_idat = 1;
        } else if (memcmp(type, "IEND", 4) == 0) {
            if (!have_ihdr || !have_idat || length != 0u)
                return IMG_ERR_SECURITY;
            return offset + 4u == size ? IMG_SUCCESS : IMG_ERR_SECURITY;
        }

        offset += (size_t)length + 4u;
    }

    return IMG_ERR_SECURITY;
}

img_result_t img_decode_stb(img_ctx_t *ctx, const uint8_t *data, size_t size, img_buffer_t *out) {
    int info_width = 0, info_height = 0, info_channels = 0;

    if (!ctx || !ctx->local_pool || !data || size == 0 || !out || size > INT_MAX)
        return IMG_ERR_SECURITY;

    const img_result_t png_validation = img_validate_png_chunks(data, size);
    if (png_validation != IMG_SUCCESS)
        return png_validation;

    if (!stbi_info_from_memory(data, (int)size, &info_width, &info_height, &info_channels))
        return IMG_ERR_FORMAT;
    if (info_width <= 0 || info_height <= 0)
        return IMG_ERR_SECURITY;

    img_result_t security_result =
        img_security_validate_request((uint32_t)info_width, (uint32_t)info_height, size);
    if (security_result != IMG_SUCCESS)
        return security_result;

    /* Reject images that cannot fit the native slab before stb allocates a
     * decoded image. This is important for large or hostile CMYK JPEG inputs
     * routed here by the TurboJPEG decoder. */
    const size_t width = (size_t)info_width;
    const size_t height = (size_t)info_height;
    if (width > SIZE_MAX / 3u)
        return IMG_ERR_SECURITY;
    const size_t row_bytes = width * 3u;
    if (height > SIZE_MAX / row_bytes)
        return IMG_ERR_SECURITY;
    const size_t required = row_bytes * height;
    const size_t block = img_slab_block_size(ctx->local_pool);
    if (block == 0)
        return IMG_ERR_SECURITY;
    if (required > block)
        return IMG_ERR_NOMEM;

    int decoded_width = 0, decoded_height = 0, decoded_channels = 0;
    uint8_t *decoded = stbi_load_from_memory(data, (int)size, &decoded_width, &decoded_height,
                                             &decoded_channels, 3);

    if (!decoded)
        return IMG_ERR_FORMAT;
    if (decoded_width != info_width || decoded_height != info_height) {
        stbi_image_free(decoded);
        return IMG_ERR_FORMAT;
    }

    uint8_t *mem = img_slab_alloc(ctx->local_pool);
    if (!mem) {
        stbi_image_free(decoded);
        return IMG_ERR_NOMEM;
    }

    memcpy(mem, decoded, required);

    // 🔥 CORRECT: construct buffer (not allocator misuse)
    out->data = mem;
    out->owner_pool = ctx->local_pool;
    out->width = (uint32_t)decoded_width;
    out->height = (uint32_t)decoded_height;
    out->channels = 3;
    out->stride = (uint32_t)row_bytes;

    stbi_image_free(decoded);
    return IMG_SUCCESS;
}
