// ./src/cmd/imgengine/main.c

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cmd/imgengine/args.h"
#include "cmd/imgengine/job_builder.h"
#include "cmd/imgengine/io_uring_engine.h"

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"

int is_pdf_output(const char *path);

static img_result_t map_readonly_file(const char *path, const uint8_t **data, size_t *size) {
    if (!path || !data || !size)
        return IMG_ERR_SECURITY;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return IMG_ERR_IO;

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return IMG_ERR_IO;
    }

    void *ptr = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
        return IMG_ERR_IO;

    *data = (const uint8_t *)ptr;
    *size = (size_t)st.st_size;
    return IMG_SUCCESS;
}

static void unmap_readonly_file(const uint8_t *data, size_t size) {
    if (data && size > 0)
        munmap((void *)data, size);
}

static img_result_t write_buffer_blocking(const char *path, const uint8_t *buf, size_t size) {
    if (!path || !buf)
        return IMG_ERR_SECURITY;

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return IMG_ERR_IO;

    size_t written = 0;
    while (written < size) {
        ssize_t rc = write(fd, buf + written, size - written);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            close(fd);
            return IMG_ERR_IO;
        }
        if (rc == 0) {
            close(fd);
            return IMG_ERR_IO;
        }
        written += (size_t)rc;
    }

    if (close(fd) != 0)
        return IMG_ERR_IO;

    return IMG_SUCCESS;
}

int main(int argc, char **argv) {
    img_cli_options_t opts = {0};

    int parse_rc = img_parse_args(argc, argv, &opts);
    if (parse_rc != 0) {
        img_print_usage(argv[0]);
        return (parse_rc > 0 || opts.help) ? 0 : 1;
    }

    img_engine_t *engine = img_api_init(opts.threads);
    if (!engine) {
        fprintf(stderr, "engine init failed\n");
        return 1;
    }

    img_job_t job;
    if (img_build_job(engine, &opts, &job) != 0) {
        fprintf(stderr, "job build failed\n");
        img_api_shutdown(engine);
        return 1;
    }

    /*
     * io_uring engine init.
     * Used for async output write — zero blocking on the critical path.
     * Depth 64: supports up to 64 concurrent I/O ops (more than enough for CLI).
     */
    img_io_uring_t uring;
    int uring_ok = (img_io_uring_init(&uring, 64) == 0);

    if (opts.verbose && !opts.quiet) {
        printf("imgengine | %ux%u grid | %.1fx%.1f cm | %u dpi | io_uring=%s\n", job.cols, job.rows,
               job.photo_w_cm, job.photo_h_cm, job.dpi, uring_ok ? "on" : "off");
        printf("input:  %s\n", opts.input_path);
        printf("output: %s\n", opts.output_path);
    }

    img_result_t r;

    if (opts.input_format == IMG_CLI_INPUT_FORMAT_RAW_RGB24) {
        const uint8_t *raw_input = NULL;
        size_t raw_size = 0;

        r = map_readonly_file(opts.input_path, &raw_input, &raw_size);
        if (r == IMG_SUCCESS) {
            uint32_t stride = opts.has_input_stride ? opts.input_stride : (opts.input_width * 3U);

            if (is_pdf_output(opts.output_path)) {
                r = img_api_run_job_rgb24(engine, raw_input, raw_size, opts.input_width,
                                          opts.input_height, stride, opts.output_path, &job);
            } else {
                uint8_t *out = NULL;
                size_t out_size = 0;

                r = img_api_run_job_rgb24_raw(engine, raw_input, raw_size, opts.input_width,
                                              opts.input_height, stride, &job, &out, &out_size);

                if (r == IMG_SUCCESS) {
                    if (uring_ok)
                        r = (img_io_uring_write_file(&uring, opts.output_path, out, out_size) == 0)
                                ? IMG_SUCCESS
                                : IMG_ERR_IO;
                    else
                        r = write_buffer_blocking(opts.output_path, out, out_size);
                }

                img_encoded_free(out);
            }

            unmap_readonly_file(raw_input, raw_size);
        }
    } else if (is_pdf_output(opts.output_path)) {
        r = img_api_run_job(engine, opts.input_path, opts.output_path, &job);
    } else {
        uint8_t *out = NULL;
        size_t out_size = 0;

        r = img_api_run_job_raw(engine, opts.input_path, &job, &out, &out_size);

        if (r == IMG_SUCCESS) {
            if (uring_ok)
                r = (img_io_uring_write_file(&uring, opts.output_path, out, out_size) == 0)
                        ? IMG_SUCCESS
                        : IMG_ERR_IO;
            else
                r = write_buffer_blocking(opts.output_path, out, out_size);
        }

        img_encoded_free(out);
    }

    if (r != IMG_SUCCESS)
        fprintf(stderr, "job failed: %s (%d)\n", img_result_name(r), r);
    else if (!opts.quiet)
        printf("done: %s\n", opts.output_path);

    img_api_shutdown(engine);
    if (uring_ok)
        img_io_uring_destroy(&uring);

    return (r == IMG_SUCCESS) ? 0 : 1;
}
