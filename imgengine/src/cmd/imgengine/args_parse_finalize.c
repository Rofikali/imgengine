// ./src/cmd/imgengine/args_parse_finalize.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>
#include <stdint.h>

int img_cli_finalize_options(img_cli_options_t *opts) {
    if (!opts)
        return -1;

    if (!opts->input_path) {
        fprintf(stderr, "imgengine: missing required --input\n");
        return -1;
    }

    if (opts->input_format == IMG_CLI_INPUT_FORMAT_RAW_RGB24) {
        if (!opts->has_input_width || !opts->has_input_height) {
            fprintf(stderr,
                    "imgengine: raw-rgb24 input requires --input-width and --input-height\n");
            return -1;
        }

        uint64_t min_stride = (uint64_t)opts->input_width * 3ULL;
        if (opts->has_input_stride && (uint64_t)opts->input_stride < min_stride) {
            fprintf(stderr, "imgengine: --input-stride must be at least width * 3 for raw-rgb24\n");
            return -1;
        }
    } else if (opts->has_input_width || opts->has_input_height || opts->has_input_stride) {
        fprintf(stderr, "imgengine: raw input dimensions require --input-format raw-rgb24\n");
        return -1;
    }

    if (!opts->has_threads)
        opts->threads = 1;

    if (opts->quiet)
        opts->verbose = false;

    return 0;
}
