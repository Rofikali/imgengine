// src/cmd/imgengine/main.c

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

typedef struct
{
    int fd;
    uint8_t *buf;
    size_t size;
} img_io_task_t;

/* Round size up to nearest multiple of align (align must be power of 2) */
static inline size_t align_up(size_t size, size_t align)
{
    return (size + align - 1) & ~(align - 1);
}

static int submit_read(img_io_uring_t *u, img_io_task_t *task)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    if (!sqe)
        return -1;

    io_uring_prep_read(sqe, task->fd, task->buf, task->size, 0);
    io_uring_sqe_set_data(sqe, task);
    return 0;
}

static int submit_write(img_io_uring_t *u, int fd, uint8_t *buf, size_t size)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&u->ring);
    if (!sqe)
        return -1;

    io_uring_prep_write(sqe, fd, buf, size, 0);
    return 0;
}

int main(int argc, char **argv)
{
    img_cli_options_t opts;

    if (img_parse_args(argc, argv, &opts) != 0)
    {
        img_print_usage(argv[0]);
        return 1;
    }

    /* ENGINE INIT */
    img_engine_t *engine = img_api_init(opts.threads);
    if (!engine)
    {
        fprintf(stderr, "engine init failed\n");
        return 1;
    }

    /* IO_URING INIT */
    img_io_uring_t u;
    if (img_io_uring_init(&u, IMG_IO_URING_DEPTH) != 0)
    {
        fprintf(stderr, "io_uring init failed\n");
        return 1;
    }

    /* OPEN INPUT FILE */
    int fd = open(opts.input_path, O_RDONLY);
    if (fd < 0)
    {
        perror("open input");
        return 1;
    }

    /* GET FILE SIZE */
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0)
    {
        fprintf(stderr, "invalid file size\n");
        close(fd);
        return 1;
    }
    lseek(fd, 0, SEEK_SET);

    /*
     * aligned_alloc() requires size to be a multiple of alignment.
     * Round up the actual file size to the next 4096-byte boundary.
     * We read only file_size bytes — the padding is unused.
     */
    const size_t ALIGN = 4096;
    size_t alloc_size = align_up((size_t)file_size, ALIGN);
    uint8_t *buffer = aligned_alloc(ALIGN, alloc_size);

    if (!buffer)
    {
        fprintf(stderr, "buffer alloc failed (size=%zu)\n", alloc_size);
        close(fd);
        return 1;
    }

    img_io_task_t task = {
        .fd = fd,
        .buf = buffer,
        .size = (size_t)file_size, /* read actual bytes, not padded size */
    };

    /* SUBMIT READ */
    submit_read(&u, &task);
    io_uring_submit(&u.ring);

    /* WAIT FOR READ COMPLETION */
    struct io_uring_cqe *cqe;
    io_uring_wait_cqe(&u.ring, &cqe);

    if (cqe->res < 0)
    {
        fprintf(stderr, "io_uring read failed: %d\n", cqe->res);
        io_uring_cqe_seen(&u.ring, cqe);
        free(buffer);
        close(fd);
        return 1;
    }
    io_uring_cqe_seen(&u.ring, cqe);

    /* PROCESS */
    uint8_t *out = NULL;
    size_t out_size = 0;

    // int ret = img_api_process_raw(engine, buffer, (size_t)file_size, &out, &out_size);
    // if (ret != 0 || !out)
    // {
    //     fprintf(stderr, "processing failed (ret=%d)\n", ret);
    //     free(buffer);
    //     close(fd);
    //     return 1;
    // }

    img_result_t ret = img_api_process_raw(
        engine, buffer, (size_t)file_size, &out, &out_size);

    if (ret != IMG_SUCCESS || !out)
    {
        fprintf(stderr, "processing failed: %s (%d)\n",
                img_result_name(ret), ret);
        free(buffer);
        close(fd);
        img_api_shutdown(engine);
        return 1;
    }

    /* WRITE OUTPUT */
    int out_fd = open(opts.output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0)
    {
        perror("open output");
        free(buffer);
        free(out);
        close(fd);
        return 1;
    }

    submit_write(&u, out_fd, out, out_size);
    io_uring_submit(&u.ring);

    io_uring_wait_cqe(&u.ring, &cqe);
    io_uring_cqe_seen(&u.ring, cqe);

    /* CLEANUP */
    close(fd);
    close(out_fd);
    free(buffer);
    free(out);

    img_io_uring_destroy(&u);
    img_api_shutdown(engine);

    if (opts.verbose)
        printf("done: %s -> %s (%zu bytes)\n",
               opts.input_path, opts.output_path, out_size);

    return 0;
}
