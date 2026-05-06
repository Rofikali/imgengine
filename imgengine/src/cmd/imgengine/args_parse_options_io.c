// ./src/cmd/imgengine/args_parse_options_io.c
#include "cmd/imgengine/args_internal.h"

int img_cli_parse_options_io(int opt, const char *optarg, img_cli_options_t *opts) {
    switch (opt) {
    case 'i':
        opts->input_path = optarg;
        return 0;
    case 'o':
        opts->output_path = optarg;
        return 0;
    case 'v':
        opts->verbose = true;
        return 0;
    case 'q':
        opts->quiet = true;
        return 0;
    case 'h':
        opts->help = true;
        return 1;
    default:
        return -1;
    }
}
