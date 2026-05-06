// ./src/cmd/imgengine/io_uring_engine.c

#define _GNU_SOURCE
#include "cmd/imgengine/io_uring_engine.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int img_io_uring_init(img_io_uring_t *u, uint32_t depth) {
    if (!u)
        return -1;
    memset(u, 0, sizeof(*u));
    return io_uring_queue_init(depth, &u->ring, 0);
}

void img_io_uring_destroy(img_io_uring_t *u) {
    if (!u)
        return;
    io_uring_queue_exit(&u->ring);
}

int img_io_uring_read_file(img_io_uring_t *u, const char *path, uint8_t **out_buf,
                           size_t *out_size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return -1;
    }

    size_t size = st.st_size;
    uint8_t *buf = malloc(size);
    if (!buf) {
        close(fd);
        return -1;
    }

    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    io_uring_prep_read(sqe, fd, buf, size, 0);

    io_uring_submit(&u->ring);

    struct io_uring_cqe *cqe;
    io_uring_wait_cqe(&u->ring, &cqe);

    if (cqe->res < 0) {
        free(buf);
        close(fd);
        return -1;
    }

    io_uring_cqe_seen(&u->ring, cqe);
    close(fd);

    *out_buf = buf;
    *out_size = size;
    return 0;
}

int img_io_uring_write_file(img_io_uring_t *u, const char *path, const uint8_t *buf, size_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    io_uring_prep_write(sqe, fd, buf, size, 0);

    io_uring_submit(&u->ring);

    struct io_uring_cqe *cqe;
    io_uring_wait_cqe(&u->ring, &cqe);

    if (cqe->res < 0) {
        close(fd);
        return -1;
    }

    io_uring_cqe_seen(&u->ring, cqe);
    close(fd);

    return 0;
}
