// src/io/decoder/streaming_decoder.c

#include "io/streaming_decoder.h"
#include "io/io_vfs.h"
#include "api/v1/img_types.h"
#include "api/v1/img_buffer_utils.h"
#include "memory/slab.h"

#include <turbojpeg.h>

#define HEADER_SIZE 1024

int img_decode_stream(
    img_ctx_t *ctx,
    img_stream_t *stream,
    img_buffer_t *out)
{
    uint8_t header[HEADER_SIZE];
    size_t read_bytes;

    // 🔥 Read only header first
    if (img_vfs_read(stream, header, HEADER_SIZE, &read_bytes) != IMG_SUCCESS)
        return -1;

    tjhandle tj = tjInitDecompress();

    int w, h, subsamp, cs;

    if (tjDecompressHeader3(tj, header, read_bytes, &w, &h, &subsamp, &cs) != 0)
    {
        tjDestroy(tj);
        return -1;
    }

    // 🔥 allocate slab
    uint8_t *mem = img_slab_alloc(ctx->local_pool);

    size_t total = w * h * 3;

    // 🔥 read FULL image now (can be optimized later)
    uint8_t *full = img_slab_alloc(ctx->local_pool);

    img_vfs_read(stream, full, total, &read_bytes);

    tjDecompress2(
        tj,
        full,
        read_bytes,
        mem,
        w,
        w * 3,
        h,
        TJPF_RGB,
        TJFLAG_FASTDCT);

    *out = img_buffer_create(mem, w, h, 3);

    tjDestroy(tj);

    return 0;
}