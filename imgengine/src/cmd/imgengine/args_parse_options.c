// ./src/cmd/imgengine/args_parse_options.c
#include "cmd/imgengine/args_internal.h"

#include <getopt.h>

static struct option long_opts[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"cols", required_argument, 0, 1},
    {"rows", required_argument, 0, 2},
    {"gap", required_argument, 0, 3},
    {"padding", required_argument, 0, 4},
    {"width", required_argument, 0, 5},
    {"height", required_argument, 0, 6},
    {"dpi", required_argument, 0, 7},
    {"border", required_argument, 0, 8},
    {"preset", required_argument, 0, 14},
    {"bleed", required_argument, 0, 9},
    {"crop-mark", required_argument, 0, 10},
    {"crop-thickness", required_argument, 0, 11},
    {"crop-offset", required_argument, 0, 12},
    {"threads", required_argument, 0, 't'},
    {"mode", required_argument, 0, 13},
    {"verbose", no_argument, 0, 'v'},
    {"quiet", no_argument, 0, 'q'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

int img_cli_parse_options(int argc, char **argv, img_cli_options_t *opts)
{
    if (!opts)
        return -1;

    int opt, idx;
    while ((opt = getopt_long(argc, argv, "i:o:t:vqh", long_opts, &idx)) != -1)
    {
        int rc;
        if (opt == 'i' || opt == 'o' || opt == 'v' || opt == 'q' || opt == 'h')
            rc = img_cli_parse_options_io(opt, optarg, opts);
        else if (opt >= 1 && opt <= 4)
            rc = img_cli_parse_options_layout(opt, optarg, opts);
        else if (opt >= 5 && opt <= 8)
            rc = img_cli_parse_options_photo(opt, optarg, opts);
        else if (opt == 14)
            rc = img_cli_parse_options_print(opt, optarg, opts);
        else
            rc = img_cli_parse_options_print(opt, optarg, opts);

        if (rc != 0)
            return rc;
    }

    return 0;
}
