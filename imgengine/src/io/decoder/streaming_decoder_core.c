// ./src/io/decoder/streaming_decoder_core.c
#define _GNU_SOURCE

#include "io/streaming_decoder_internal.h"
#include "io/io_vfs.h"

#include "core/buffer.h"
#include "memory/slab.h"
#include "security/input_validator.h"
#include "security/bounds_check.h"

#include "core/result.h"

#include <turbojpeg.h>

#include <stdlib.h>
#include <stdio.h>

#define HEADER_SIZE 8192

img_result_t img_decode_stream_core(img_slab_pool_t *pool, img_stream_t *stream,
                                    img_buffer_t *out) {
    if (!pool || !stream || !out)
        return IMG_ERR_INTERNAL;

    uint8_t header[HEADER_SIZE];
    size_t read_bytes = 0;

    img_result_t rc = img_vfs_read(stream, header, HEADER_SIZE, &read_bytes);
    if (rc != IMG_SUCCESS)
        return rc;

    tjhandle tj = tjInitDecompress();
    if (!tj)
        return IMG_ERR_NOMEM;

    int w = 0, h = 0, subsamp = 0, cs = 0;

    if (tjDecompressHeader3(tj, header, read_bytes, &w, &h, &subsamp, &cs) != 0) {
        tjDestroy(tj);
        return IMG_ERR_FORMAT;
    }

    (void)w; (void)h; (void)stream; /* debug prints removed; use observability logger if needed */

    img_result_t sec = img_security_validate_request(w, h, stream->size);
    if (sec != IMG_SUCCESS) {
        tjDestroy(tj);
        return sec;
    }

    size_t stride = (size_t)w * 3;
    size_t required = stride * (size_t)h;

    size_t block = img_slab_block_size(pool);
    if (required > block) {
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    /* Ensure compressed input fits in one slab block as well */
    if (stream->size > block) {
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem) {
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    uint8_t *tmp = img_slab_alloc(pool);
    if (!tmp) {
        img_slab_free(pool, mem);
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    /* Read the full compressed JPEG blob (reset pos to include header) */
    stream->pos = 0;
    rc = img_vfs_read(stream, tmp, stream->size, &read_bytes);
    if (rc != IMG_SUCCESS) {
        img_slab_free(pool, tmp);
        img_slab_free(pool, mem);
        tjDestroy(tj);
        return rc;
    }

    /* tmp contains a copy of the compressed blob; ensure we read something sensible */
    if (read_bytes == 0 || read_bytes > stream->size) {
        img_slab_free(pool, tmp);
        img_slab_free(pool, mem);
        tjDestroy(tj);
        return IMG_ERR_FORMAT;
    }

    if (tjDecompress2(tj, tmp, read_bytes, mem, w, stride, h, TJPF_RGB,
                      TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) != 0) {
        img_slab_free(pool, tmp);
        img_slab_free(pool, mem);
        tjDestroy(tj);
        return IMG_ERR_FORMAT;
    }

    out->data = mem;
    out->owner_pool = pool;
    out->width = (uint32_t)w;
    out->height = (uint32_t)h;
    out->channels = 3;
    out->stride = (uint32_t)stride;

    img_slab_free(pool, tmp);
    tjDestroy(tj);
    return IMG_SUCCESS;
}
