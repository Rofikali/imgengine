// ./src/cmd/imgengine/args.c

#include "cmd/imgengine/args.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    {"bleed", required_argument, 0, 9},
    {"crop-mark", required_argument, 0, 10},
    {"crop-thickness", required_argument, 0, 11},
    {"crop-offset", required_argument, 0, 12},

    {"threads", required_argument, 0, 't'},
    {"verbose", no_argument, 0, 'v'},

    {0, 0, 0, 0}};

int img_parse_args(int argc, char **argv, img_cli_options_t *opts)
{
    if (!opts)
        return -1;

    memset(opts, 0, sizeof(*opts));

    int opt, idx;

    while ((opt = getopt_long(argc, argv, "i:o:w:h:t:bv?", long_opts, &idx)) != -1)
    {
        switch (opt)
        {
        case 'i':
            opts->input_path = optarg;
            break;
        case 'o':
            opts->output_path = optarg;
            break;

        case 1:
            opts->cols = atoi(optarg);
            break;
        case 2:
            opts->rows = atoi(optarg);
            break;
        case 3:
            opts->gap = atoi(optarg);
            break;
        case 4:
            opts->padding = atoi(optarg);
            break;

        case 5:
            opts->photo_w_cm = atof(optarg);
            break;
        case 6:
            opts->photo_h_cm = atof(optarg);
            break;
        case 7:
            opts->dpi = atoi(optarg);
            break;
        case 8:
            opts->border = atoi(optarg);
            break;

        case 9:
            opts->bleed = atoi(optarg);
            break;
        case 10:
            opts->crop_mark = atoi(optarg);
            break;
        case 11:
            opts->crop_thickness = atoi(optarg);
            break;
        case 12:
            opts->crop_offset = atoi(optarg);
            break;

        case 't':
            opts->threads = atoi(optarg);
            break;
        case 'v':
            opts->verbose = true;
            break;
        }
    }

    if (!opts->input_path || !opts->output_path)
        return -1;

    if (opts->threads == 0)
        opts->threads = 1;

    return 0;
}

void img_print_usage(const char *bin)
{
    printf("\nIMGENGINE CLI\n");
    printf("Professional Photo Layout & Print Pipeline\n");
    printf("------------------------------------------------------------\n\n");

    printf("USAGE:\n  %s --input <file> [--output <file>] [OPTIONS]\n\n", bin);

    printf("REQUIRED:\n");
    printf("  --input <file>         Input image (jpg/png)\n\n");

    printf("OUTPUT:\n");
    printf("  --output <file>        Output file .jpg/.png/.pdf (default: output.jpg)\n\n");

    printf("LAYOUT OPTIONS:\n");
    printf("  --cols <int>           Columns (default: 2)\n");
    printf("  --rows <int>           Rows    (default: 3)\n");
    printf("  --gap <px>             Gap between photos (default: 15)\n");
    printf("  --padding <px>         Page margin (default: 20)\n\n");

    printf("PHOTO SETTINGS:\n");
    printf("  --width <cm>           Photo width  in cm (default: 4.5)\n");
    printf("  --height <cm>          Photo height in cm (default: 3.5)\n");
    printf("  --dpi <int>            Print DPI (default: 300)\n");
    printf("  --border <px>          Border thickness (default: 2)\n\n");

    printf("PRINT FEATURES:\n");
    printf("  --bleed <px>           Edge bleed (default: 10)\n");
    printf("  --crop-mark <px>       Mark length (default: 20)\n");
    printf("  --crop-thickness <px>  Mark thickness (default: 2)\n");
    printf("  --crop-offset <px>     Distance from image edge (default: 8)\n\n");

    printf("OTHER:\n");
    printf("  --threads <int>        Worker threads (default: 1)\n");
    printf("  --quiet                Suppress output\n");
    printf("  --help                 Show this help\n\n");

    printf("EXAMPLES:\n\n");
    printf("  Passport sheet:\n");
    printf("    %s --input photo.jpg --cols 3 --rows 2\n\n", bin);
    printf("  Print-ready A4 with crop marks:\n");
    printf("    %s --input photo.jpg --output sheet.jpg \\\n", bin);
    printf("        --cols 6 --rows 2 --width 3.5 --height 3.0 \\\n");
    printf("        --bleed 10 --crop-mark 25 --crop-offset 8\n\n");
    printf("  PDF export:\n");
    printf("    %s --input photo.jpg --output sheet.pdf --dpi 300\n\n", bin);
}
