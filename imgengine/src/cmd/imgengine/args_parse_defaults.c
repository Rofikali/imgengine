// ./src/cmd/imgengine/args_parse_defaults.c
#include "cmd/imgengine/args_internal.h"

#include <string.h>

void img_cli_set_defaults(img_cli_options_t *opts)
{
    if (!opts)
        return;

    memset(opts, 0, sizeof(*opts));
    opts->output_path = "output.jpg";
    opts->threads = 1;
    opts->mode = IMG_FILL;
    opts->has_threads = true;
    opts->preset_template = IMG_JOB_TEMPLATE_CUSTOM;
}
