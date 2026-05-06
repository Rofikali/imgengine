// ./src/cmd/imgengine/args_parse_options_input.c
#include "cmd/imgengine/args_internal.h"

#include <string.h>

int img_cli_parse_options_input(int opt, const char *optarg, img_cli_options_t *opts) {
    switch (opt) {
    case 15:
        if (!optarg)
            return -1;
        if (strcmp(optarg, "encoded") == 0) {
            opts->input_format = IMG_CLI_INPUT_FORMAT_ENCODED;
            return 0;
        }
        if (strcmp(optarg, "raw-rgb24") == 0) {
            opts->input_format = IMG_CLI_INPUT_FORMAT_RAW_RGB24;
            return 0;
        }
        return -1;
    case 16:
        if (img_cli_parse_u32(optarg, &opts->input_width, "input-width") != 0)
            return -1;
        opts->has_input_width = true;
        return 0;
    case 17:
        if (img_cli_parse_u32(optarg, &opts->input_height, "input-height") != 0)
            return -1;
        opts->has_input_height = true;
        return 0;
    case 18:
        if (img_cli_parse_u32(optarg, &opts->input_stride, "input-stride") != 0)
            return -1;
        opts->has_input_stride = true;
        return 0;
    default:
        return -1;
    }
}
