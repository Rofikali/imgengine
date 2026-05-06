// ./src/io/vfs/http_stream.c

// ./src/io/vfs/http_stream.c

#include "io/remote_fetch.h"
#include "io/io_vfs.h"
#include <string.h>

// 🔥 HTTP-backed stream
typedef struct {
    const char *url;
    size_t pos;
} http_stream_t;

img_result_t img_vfs_http_read(http_stream_t *stream, uint8_t *dst, size_t n, size_t *read_bytes) {
    size_t end = stream->pos + n;

    int ret = img_http_fetch_range(stream->url, stream->pos, end, dst, read_bytes);

    if (ret != 0)
        return IMG_ERR_IO;

    stream->pos += *read_bytes;

    return IMG_SUCCESS;
}

/* FILE: src/io/vfs/http_stream.c — add this function */

/*
 * img_http_fetch_range()
 *
 * Stub: returns error until libcurl integration is wired.
 * Caller (http_stream) falls back to full fetch on non-zero return.
 */
int img_http_fetch_range(const char *url, size_t start, size_t end, uint8_t *buffer,
                         size_t *received) {
    (void)url;
    (void)start;
    (void)end;
    (void)buffer;

    if (received)
        *received = 0;

    return -1; /* not yet implemented */
}

/*
 * img_http_fetch_full()
 *
 * Stub: same — not yet implemented.
 */
int img_http_fetch_full(const char *url, uint8_t **data, size_t *size) {
    (void)url;

    if (data)
        *data = NULL;
    if (size)
        *size = 0;

    return -1;
}