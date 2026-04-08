// cmd/imgengine/main.c

#define _GNU_SOURCE

#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd/imgengine/args.h"
#include "cmd/imgengine/io_uring_engine.h"
#include "api/v1/img_api.h"

#include "observability/logger/logger.h"

// ============================================================
// 🔥 IO TASK
// ============================================================

typedef struct
{
    int fd;
    uint8_t *buf;
    size_t size;
} img_io_task_t;

// ============================================================
// 🔥 READ SUBMIT
// ============================================================

static int submit_read(img_io_uring_t *u, img_io_task_t *task)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    if (!sqe)
        return -1;

    io_uring_prep_read(sqe, task->fd, task->buf, task->size, 0);
    io_uring_sqe_set_data(sqe, task);

    return 0;
}

// ============================================================
// 🔥 WRITE SUBMIT
// ============================================================

static int submit_write(img_io_uring_t *u, int fd, uint8_t *buf, size_t size)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    if (!sqe)
        return -1;

    io_uring_prep_write(sqe, fd, buf, size, 0);

    return 0;
}

// ============================================================
// 🔥 FILE SIZE
// ============================================================

static size_t get_file_size(int fd)
{
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return (size_t)size;
}

// ============================================================
// 🔥 MAIN
// ============================================================

int main(int argc, char **argv)
{
    img_cli_options_t opts;

    if (img_parse_args(argc, argv, &opts) != 0)
    {
        img_print_usage(argv[0]);
        return 1;
    }

    img_logger_init();

    // ================= ENGINE =================
    img_engine_t *engine = img_api_init(opts.threads);
    if (!engine)
    {
        printf("engine init failed\n");
        return 1;
    }

    // ================= IO_URING =================
    img_io_uring_t u;
    if (img_io_uring_init(&u, IMG_IO_URING_DEPTH) != 0)
    {
        printf("io_uring init failed\n");
        return 1;
    }

    // ================= INPUT =================
    int in_fd = open(opts.input_path, O_RDONLY);
    if (in_fd < 0)
    {
        perror("open input");
        return 1;
    }

    size_t size = get_file_size(in_fd);

    // 🔥 aligned buffer (DMA friendly)
    uint8_t *buffer = aligned_alloc(4096, size);
    if (!buffer)
        return 1;

    img_io_task_t task = {
        .fd = in_fd,
        .buf = buffer,
        .size = size};

    // ================= READ =================
    submit_read(&u, &task);
    io_uring_submit(&u.ring);

    struct io_uring_cqe *cqe;
    io_uring_wait_cqe(&u.ring, &cqe);

    if (cqe->res < 0)
    {
        printf("read failed\n");
        return 1;
    }

    io_uring_cqe_seen(&u.ring, cqe);

    // ================= PROCESS =================
    uint8_t *out = NULL;
    size_t out_size = 0;

    int ret = img_api_process_raw(
        engine,
        buffer,
        size,
        &out,
        &out_size);

    if (ret != IMG_SUCCESS)
    {
        printf("processing failed: %d\n", ret);
        return 1;
    }

    // ================= OUTPUT =================
    int out_fd = open(opts.output_path,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);

    if (out_fd < 0)
    {
        perror("open output");
        return 1;
    }

    submit_write(&u, out_fd, out, out_size);
    io_uring_submit(&u.ring);

    io_uring_wait_cqe(&u.ring, &cqe);
    io_uring_cqe_seen(&u.ring, cqe);

    // ================= CLEANUP =================
    close(in_fd);
    close(out_fd);

    free(buffer);

    img_io_uring_destroy(&u);
    img_api_shutdown(engine);
    img_logger_shutdown();

    return 0;
}