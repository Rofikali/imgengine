// src/cmd/imgengine/main.c

// Production CLI — all flags from original design, new unified API.

#define _GNU_SOURCE

#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api/v1/img_api.h"
#include "api/v1/img_job.h"
#include "api/v1/img_error.h"

static void print_usage(const char *bin)
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

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        return 0;
    }

    const char *input = NULL;
    const char *output = "output.jpg";
    uint32_t threads = 1;
    int verbose = 1;

    img_job_t job;
    img_job_defaults(&job);

    /* ── Argument parser ── */
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--input") && i + 1 < argc)
            input = argv[++i];
        else if (!strcmp(argv[i], "--output") && i + 1 < argc)
            output = argv[++i];
        else if (!strcmp(argv[i], "--cols") && i + 1 < argc)
            job.cols = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--rows") && i + 1 < argc)
            job.rows = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--gap") && i + 1 < argc)
            job.gap = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--dpi") && i + 1 < argc)
            job.dpi = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--width") && i + 1 < argc)
            job.photo_w_cm = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "--height") && i + 1 < argc)
            job.photo_h_cm = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "--border") && i + 1 < argc)
            job.border_px = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--padding") && i + 1 < argc)
            job.padding = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--bleed") && i + 1 < argc)
            job.bleed_px = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--crop-mark") && i + 1 < argc)
            job.crop_mark_px = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--crop-thickness") && i + 1 < argc)
            job.crop_thickness = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--crop-offset") && i + 1 < argc)
            job.crop_offset_px = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--threads") && i + 1 < argc)
            threads = (uint32_t)atoi(argv[++i]);
        else if (!strcmp(argv[i], "--quiet"))
            verbose = 0;
        else if (!strcmp(argv[i], "--help"))
        {
            print_usage(argv[0]);
            return 0;
        }
        else
        {
            fprintf(stderr, "unknown argument: %s\n\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* ── Validation ── */
    if (!input)
    {
        fprintf(stderr, "error: --input is required\n");
        return 1;
    }
    if (job.cols == 0 || job.rows == 0)
    {
        fprintf(stderr, "error: cols and rows must be > 0\n");
        return 1;
    }
    if (job.dpi == 0)
    {
        fprintf(stderr, "error: dpi must be > 0\n");
        return 1;
    }
    if (job.photo_w_cm <= 0.0f || job.photo_h_cm <= 0.0f)
    {
        fprintf(stderr, "error: width/height must be > 0\n");
        return 1;
    }

    /* ── Engine init ── */
    img_engine_t *engine = img_api_init(threads);
    if (!engine)
    {
        fprintf(stderr, "engine init failed\n");
        return 1;
    }

    if (verbose)
    {
        printf("imgengine | %dx%d grid | %.1fx%.1f cm | %u dpi\n",
               job.cols, job.rows,
               job.photo_w_cm, job.photo_h_cm,
               job.dpi, job.crop_mark_px,
               job.crop_thickness, job.padding,
               job.bleed_px, job.abi_version);
        printf("input:  %s\n", input);
        printf("output: %s\n", output);
    }

    /* ── Run job ── */
    img_result_t r = img_api_run_job(engine, input, output, &job);

    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "job failed: %s (%d)\n", img_result_name(r), r);
    }
    else if (verbose)
    {
        printf("done: %s\n", output);
    }

    img_api_shutdown(engine);
    return (r == IMG_SUCCESS) ? 0 : 1;
}
