// src/io/vfs/http_stream.c

#include "io/remote_fetch.h"
#include "io/io_vfs.h"
#include <string.h>

// 🔥 HTTP-backed stream
typedef struct
{
    const char *url;
    size_t pos;
} http_stream_t;

img_result_t img_vfs_http_read(
    http_stream_t *stream,
    uint8_t *dst,
    size_t n,
    size_t *read_bytes)
{
    size_t end = stream->pos + n;

    int ret = img_http_fetch_range(
        stream->url,
        stream->pos,
        end,
        dst,
        read_bytes);

    if (ret != 0)
        return IMG_ERR_IO;

    stream->pos += *read_bytes;

    return IMG_SUCCESS;
}