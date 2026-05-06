// ./src/cmd/imgengine/args_parse_options_photo.c
#include "cmd/imgengine/args_internal.h"

int img_cli_parse_options_photo(int opt, const char *optarg, img_cli_options_t *opts) {
    switch (opt) {
    case 5:
        if (img_cli_parse_float(optarg, &opts->photo_w_cm, "width") != 0)
            return -1;
        opts->has_photo_w_cm = true;
        return 0;
    case 6:
        if (img_cli_parse_float(optarg, &opts->photo_h_cm, "height") != 0)
            return -1;
        opts->has_photo_h_cm = true;
        return 0;
    case 7:
        if (img_cli_parse_u32(optarg, &opts->dpi, "dpi") != 0)
            return -1;
        opts->has_dpi = true;
        return 0;
    case 8:
        if (img_cli_parse_u32(optarg, &opts->border, "border") != 0)
            return -1;
        opts->has_border = true;
        return 0;
    default:
        return -1;
    }
}
