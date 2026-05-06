// ./src/cmd/imgengine/args_parse_options_layout.c
#include "cmd/imgengine/args_internal.h"

int img_cli_parse_options_layout(int opt, const char *optarg, img_cli_options_t *opts) {
    switch (opt) {
    case 1:
        if (img_cli_parse_u32(optarg, &opts->cols, "cols") != 0)
            return -1;
        opts->has_cols = true;
        return 0;
    case 2:
        if (img_cli_parse_u32(optarg, &opts->rows, "rows") != 0)
            return -1;
        opts->has_rows = true;
        return 0;
    case 3:
        if (img_cli_parse_u32(optarg, &opts->gap, "gap") != 0)
            return -1;
        opts->has_gap = true;
        return 0;
    case 4:
        if (img_cli_parse_u32(optarg, &opts->padding, "padding") != 0)
            return -1;
        opts->has_padding = true;
        return 0;
    default:
        return -1;
    }
}
