
// io/io_vsf.h

#ifndef IMGENGINE_IO_VFS_H
#define IMGENGINE_IO_VFS_H

#include <stdint.h>
#include <stddef.h>
#include "api/v1/img_error.h"

/*
 * 🔥 REAL STRUCT NAME (MATCH FORWARD DECL)
 */
typedef struct img_stream
{
    const uint8_t *data;
    size_t size;
    size_t pos;
} img_stream_t;

/*
 * 🔥 API
 */
img_result_t img_vfs_open_mem(
    img_stream_t *stream,
    const uint8_t *data,
    size_t size);

img_result_t img_vfs_read(
    img_stream_t *stream,
    uint8_t *dst,
    size_t n,
    size_t *read_bytes);

#endif