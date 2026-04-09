// cmd/imgengine/io_uring_engine.c

#define _GNU_SOURCE

#include "cmd/imgengine/io_uring_engine.h"
#include <string.h>

int img_io_uring_init(img_io_uring_t *u, uint32_t depth)
{
    if (!u)
        return -1;

    memset(u, 0, sizeof(*u));

    return io_uring_queue_init(depth, &u->ring, 0);
}

void img_io_uring_destroy(img_io_uring_t *u)
{
    if (!u)
        return;

    io_uring_queue_exit(&u->ring);
}