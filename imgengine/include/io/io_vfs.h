// io/io_vsf.h

#ifndef IMGENGINE_IO_VFS_H
#define IMGENGINE_IO_VFS_H

#include <stdint.h>
#include <stddef.h>
#include "api/v1/img_error.h"

typedef struct
{
    const uint8_t *data;
    size_t size;
    size_t pos;
} img_stream_t;

// Memory-backed stream
img_result_t img_vfs_open_mem(
    img_stream_t *stream,
    const uint8_t *data,
    size_t size);

// Safe read
img_result_t img_vfs_read(
    img_stream_t *stream,
    uint8_t *dst,
    size_t n,
    size_t *read_bytes);

#endif