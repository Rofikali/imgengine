// ./src/io/vfs/memory_stream.c

// ./src/io/vfs/memory_stream.c



#include "io/io_vfs.h"
#include <string.h>

img_result_t img_vfs_open_mem(
    img_stream_t *stream,
    const uint8_t *data,
    size_t size)
{
    if (!stream || !data || size == 0)
        return IMG_ERR_FORMAT;

    stream->data = data;
    stream->size = size;
    stream->pos = 0;

    return IMG_SUCCESS;
}

img_result_t img_vfs_read(
    img_stream_t *stream,
    uint8_t *dst,
    size_t n,
    size_t *read_bytes)
{
    size_t available = stream->size - stream->pos;
    size_t to_copy = (n < available) ? n : available;

    if (to_copy > 0)
    {
        memcpy(dst, stream->data + stream->pos, to_copy);
        stream->pos += to_copy;
    }

    if (read_bytes)
        *read_bytes = to_copy;

    return (to_copy == n) ? IMG_SUCCESS : IMG_ERR_FORMAT;
}