// ./src/cmd/imgengine/args.c

#include "cmd/imgengine/args.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct option long_opts[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"width", required_argument, 0, 'w'},
    {"height", required_argument, 0, 'h'},
    {"threads", required_argument, 0, 't'},
    {"bench", no_argument, 0, 'b'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, '?'},
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
        case 'w':
            opts->width = atoi(optarg);
            break;
        case 'h':
            opts->height = atoi(optarg);
            break;
        case 't':
            opts->threads = atoi(optarg);
            break;
        case 'b':
            opts->bench_mode = true;
            break;
        case 'v':
            opts->verbose = true;
            break;

        default:
            return -1;
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
    printf("Usage:\n");
    printf("  %s -i input.jpg -o output.jpg [options]\n\n", bin);
    printf("Options:\n");
    printf("  -w --width <n>\n");
    printf("  -h --height <n>\n");
    printf("  -t --threads <n>\n");
    printf("  -b --bench\n");
    printf("  -v --verbose\n");
}

// I want this.
// static void print_usage() {
//     printf("\nIMGENGINE CLI\n");
//     printf("Professional Photo Layout & Print Pipeline\n");
//     printf("------------------------------------------------------------\n\n");
//     printf("USAGE:\n  imgengine_cli --input <file> [--output <file>] [OPTIONS]\n\n");
//     printf("REQUIRED:\n  --input <file>         Input image (jpg/png)\n\n");
//     printf("LAYOUT OPTIONS:\n");
//     printf("  --cols <int>           Number of columns (default: %d)\n", DEFAULT_COLS);
//     printf("  --rows <int>           Number of rows (default: %d)\n", DEFAULT_ROWS);
//     printf("  --gap <px>             Gap between photos (default: %d)\n", DEFAULT_GAP);
//     printf("  --padding <px>         Page margin (default: %d)\n", DEFAULT_PADDING);
//     printf("\nPRINT FEATURES:\n");
//     printf("  --bleed <px>           Edge bleed (default: %d)\n", DEFAULT_BLEED);
//     printf("  --crop-mark <px>       Mark length (default: %d)\n", DEFAULT_CROP_MARK);
//     printf("  --crop-offset <px>     Distance from edge (default: %d)\n", DEFAULT_CROP_OFFSET);

//     printf("PHOTO SETTINGS:\n");
//     printf("  --width <cm>           Photo width in cm (default: %.1f)\n", DEFAULT_W_CM);
//     printf("  --height <cm>          Photo height in cm (default: %.1f)\n", DEFAULT_H_CM);
//     printf("  --dpi <int>            Print DPI (default: %d)\n", DEFAULT_DPI);
//     printf("  --border <px>          Border thickness (default: %d)\n", DEFAULT_BORDER);
//     printf("\n");

//     printf("\nEXAMPLES:\n");
//     printf(
//         "  ./imgengine_cli --input ../input.jpg --output final_output.png --cols 6 --rows 2 --gap \
//         15 --width 3.5 --height 3.0 --padding 20 --bleed 10 --crop-mark 25 --crop-offset 8 \n\n");
// }