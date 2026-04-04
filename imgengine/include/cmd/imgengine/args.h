/* cmd/imgengine/args.h */

#ifndef IMGENGINE_CLI_ARGS_H
#define IMGENGINE_CLI_ARGS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    const char *input_path;
    const char *output_path;
    uint32_t width;
    uint32_t height;
    uint32_t threads;
    bool verbose;
    bool bench_mode;
} img_cli_options_t;

int img_parse_args(int argc, char **argv, img_cli_options_t *opts);
void img_print_usage(const char *bin_name);

#endif
