
// include/cmd/imgengine/io_uring_engine.h

#ifndef IMGENGINE_IO_URING_ENGINE_H
#define IMGENGINE_IO_URING_ENGINE_H

#include <liburing.h>
#include <stdint.h>

#define IMG_IO_URING_DEPTH 256
#define IMG_MAX_BATCH 64

typedef struct
{
    struct io_uring ring;

} img_io_uring_t;

int img_io_uring_init(img_io_uring_t *u, uint32_t depth);
void img_io_uring_destroy(img_io_uring_t *u);

#endif