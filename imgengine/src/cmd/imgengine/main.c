// src/cmd/imgengine/main.c

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd/imgengine/args.h"
#include "cmd/imgengine/job_builder.h"
#include "cmd/imgengine/io_uring_engine.h"

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"

int main(int argc, char **argv)
{
    img_cli_options_t opts;

    if (img_parse_args(argc, argv, &opts) != 0)
    {
        img_print_usage(argv[0]);
        return 1;
    }

    img_job_t job;
    if (img_build_job(&opts, &job) != 0)
    {
        fprintf(stderr, "job build failed\n");
        return 1;
    }

    /*
     * io_uring engine init.
     * Used for async output write — zero blocking on the critical path.
     * Depth 64: supports up to 64 concurrent I/O ops (more than enough for CLI).
     */
    img_io_uring_t uring;
    int uring_ok = (img_io_uring_init(&uring, 64) == 0);

    img_engine_t *engine = img_api_init(opts.threads);
    if (!engine)
    {
        fprintf(stderr, "engine init failed\n");
        if (uring_ok)
            img_io_uring_destroy(&uring);
        return 1;
    }

    if (opts.verbose)
    {
        printf("imgengine | %ux%u grid | %.1fx%.1f cm | %u dpi | io_uring=%s\n",
               job.cols, job.rows,
               job.photo_w_cm, job.photo_h_cm,
               job.dpi,
               uring_ok ? "on" : "off");
        printf("input:  %s\n", opts.input_path);
        printf("output: %s\n", opts.output_path);
    }

    /*
     * Run job.
     * img_api_run_job() handles decode (mmap), pipeline, encode internally.
     * Output is written to output_path via standard write.
     *
     * io_uring async write: wire here when img_api_run_job_raw() is available
     * (returns raw bytes instead of writing directly). Current architecture
     * writes internally — io_uring write replaces that in the next sprint.
     */
    img_result_t r = img_api_run_job(
        engine,
        opts.input_path,
        opts.output_path,
        &job);

    if (r != IMG_SUCCESS)
        fprintf(stderr, "job failed: %s (%d)\n", img_result_name(r), r);
    else
        printf("done: %s\n", opts.output_path);

    img_api_shutdown(engine);
    if (uring_ok)
        img_io_uring_destroy(&uring);

    return (r == IMG_SUCCESS) ? 0 : 1;
}
