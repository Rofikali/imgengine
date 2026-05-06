// ./src/cmd/imgengine/args_parse.c
#include "cmd/imgengine/args_internal.h"

int img_parse_args(int argc, char **argv, img_cli_options_t *opts) {
    if (!opts)
        return -1;

    img_cli_set_defaults(opts);

    int parse_rc = img_cli_parse_options(argc, argv, opts);
    if (parse_rc != 0)
        return parse_rc;

    return img_cli_finalize_options(opts);
}
