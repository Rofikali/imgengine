// io/decoder/decoder_entry.c

#include "io/decoder/decoder_entry.h"
#include <turbojpeg.h>
#include <string.h>

int img_decode_to_buffer(img_ctx_t *ctx,
                         uint8_t *input,
                         size_t size,
                         img_buffer_t *out)
{
    if (!ctx || !input || !out)
        return -1;

    tjhandle tj = tjInitDecompress();
    // tjhandle tj = ctx->tj_decoder;

    if (!tj)
        return -1;

    int width, height, subsamp, colorspace;

    // 🔥 1. Read JPEG header (no decode yet)
    if (tjDecompressHeader3(tj,
                            input,
                            size,
                            &width,
                            &height,
                            &subsamp,
                            &colorspace) != 0)
    {
        tjDestroy(tj);
        return -1;
    }

    // 🔥 2. Allocate SLAB memory (ZERO COPY TARGET)
    int channels = 3; // RGB
    size_t stride = width * channels;
    size_t required_size = stride * height;

    uint8_t *mem = img_slab_alloc(ctx->local_pool);
    if (!mem)
    {
        tjDestroy(tj);
        return -1;
    }

    // ⚠️ (Optional safety check)
    // ensure slab block is big enough
    // (you can enforce via block_size config)

    // 🔥 3. DECODE DIRECTLY INTO SLAB MEMORY
    if (tjDecompress2(tj,
                      input,
                      size,
                      mem,
                      width,
                      stride,
                      height,
                      TJPF_RGB, // RGB format
                      TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) != 0)
    {
        img_slab_free(ctx->local_pool, mem);
        tjDestroy(tj);
        return -1;
    }

    // 🔥 4. Wrap into img_buffer (ZERO COPY)
    *out = img_buffer_create(
        mem,
        width,
        height,
        channels);

    tjDestroy(tj);
    return 0;
}

// #include "core/image.h"
// #include "memory/slab.h"
// #include "core/context.h"
// #include <turbojpeg.h>
// #include <string.h>

// int img_decode_to_buffer(img_ctx_t *ctx,
//                          uint8_t *input,
//                          size_t size,
//                          img_buffer_t *out)
// {
//     // 🔥 allocate slab memory
//     uint8_t *mem = img_slab_alloc(ctx->pool);
//     if (!mem)
//         return -1;

//     // Example: JPEG (you’ll plug turbojpeg later)
//     // For now assume raw RGB

//     *out = img_buffer_create(
//         mem,
//         1024, // placeholder width
//         1024, // placeholder height
//         3     // channels
//     );

//     // TODO: real decode → write directly into mem

//     return 0;
// }