// ./include/cmd/imgengine/io_uring_engine.h

#ifndef IMGENGINE_IO_URING_ENGINE_H
#define IMGENGINE_IO_URING_ENGINE_H

#include <liburing.h>
#include <stdint.h>

typedef struct
{
    struct io_uring ring;
} img_io_uring_t;

int img_io_uring_init(img_io_uring_t *u, uint32_t depth);
void img_io_uring_destroy(img_io_uring_t *u);

/* NEW */
int img_io_uring_read_file(
    img_io_uring_t *u,
    const char *path,
    uint8_t **out_buf,
    size_t *out_size);

int img_io_uring_write_file(
    img_io_uring_t *u,
    const char *path,
    const uint8_t *buf,
    size_t size);

#endif
