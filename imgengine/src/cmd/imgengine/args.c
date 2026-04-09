// cmd/imgengine/args.c

#include "cmd/imgengine/args.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================
// 🔥 OPTIONS TABLE
// ============================================================

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

// ============================================================
// 🔥 PARSER
// ============================================================

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
            opts->width = (uint32_t)atoi(optarg);
            break;

        case 'h':
            opts->height = (uint32_t)atoi(optarg);
            break;

        case 't':
            opts->threads = (uint32_t)atoi(optarg);
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

    // ================= VALIDATION =================

    if (!opts->input_path || !opts->output_path)
        return -1;

    if (opts->threads == 0)
        opts->threads = 1;

    if (opts->threads > 64)
        opts->threads = 64;

    return 0;
}

// ============================================================
// 🔥 USAGE
// ============================================================

void img_print_usage(const char *bin)
{
    printf("Usage:\n");
    printf("  %s -i input.jpg -o output.jpg [options]\n\n", bin);

    printf("Options:\n");
    printf("  -i --input <file>\n");
    printf("  -o --output <file>\n");
    printf("  -w --width <n>\n");
    printf("  -h --height <n>\n");
    printf("  -t --threads <n>\n");
    printf("  -b --bench\n");
    printf("  -v --verbose\n");
}