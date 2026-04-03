io/decoder/jpeg/

#include <turbojpeg.h>
#include "api/v1/img_error.h"
#include "core/image.h"
#include "core/context.h"
#include "io/io_vfs.h"

img_result_t img_decode_jpeg(
    img_ctx_t *ctx,
    img_stream_t *stream,
    img_buffer_t *out)
{
    tjhandle handle = tjInitDecompress();
    if (!handle)
        return IMG_ERR_IO;

    int w, h, subsamp, colorspace;

    // 1. FAST HEADER READ
    if (tjDecompressHeader3(
            handle,
            stream->data,
            stream->size,
            &w, &h,
            &subsamp,
            &colorspace) != 0)
    {
        tjDestroy(handle);
        return IMG_ERR_FORMAT;
    }

    // 2. SECURITY LIMIT (64MB)
    if ((uint64_t)w * h * 4 > 64 * 1024 * 1024)
    {
        tjDestroy(handle);
        return IMG_ERR_NOMEM;
    }

    // 3. ALLOCATE SLAB
    uint8_t *mem = img_slab_alloc(ctx->pool);
    if (!mem)
    {
        tjDestroy(handle);
        return IMG_ERR_NOMEM;
    }

    *out = img_buffer_create(mem, w, h, 3);

    // 4. DECODE DIRECT → SLAB
    if (tjDecompress2(
            handle,
            stream->data,
            stream->size,
            out->data,
            w,
            out->stride,
            h,
            TJPF_RGB,
            TJFLAG_FASTDCT) != 0)
    {
        tjDestroy(handle);
        return IMG_ERR_FORMAT;
    }

    tjDestroy(handle);
    return IMG_SUCCESS;
}