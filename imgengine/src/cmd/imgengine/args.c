/* cmd/imgengine/args.c */
#include "cmd/imgengine/args.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"width", required_argument, 0, 'w'},
    {"height", required_argument, 0, 'h'},
    {"threads", required_argument, 0, 't'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, '?'},
    {0, 0, 0, 0}};

int img_parse_args(int argc, char **argv, img_cli_options_t *opts)
{
    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:w:h:t:v?", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'i':
            opts->input_path = optarg;
            break;
        case 'o':
            opts->output_path = optarg;
            break;
        case 'w':
            opts->width = atoi(optarg);
            break;
        case 'h':
            opts->height = atoi(optarg);
            break;
        case 't':
            opts->threads = atoi(optarg);
            break;
        case 'v':
            opts->verbose = true;
            break;
        default:
            return -1;
        }
    }
    return (opts->input_path && opts->output_path) ? 0 : -1;
}

void img_print_usage(const char *bin)
{
    printf("Usage: %s -i <in.jpg> -o <out.jpg> -w <width> -h <height> [options]\n", bin);
}
