// ./cli/main.c

#include "imgengine/api.h"
#include "imgengine/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * PRODUCTION CONSTANTS
 * 100MB is the optimized slab size for A4 @ 300DPI processing.
 */
#define POOL_SIZE_MB 100
#define POOL_SIZE (POOL_SIZE_MB * 1024 * 1024)

#define DEFAULT_DPI 300
#define DEFAULT_COLS 2
#define DEFAULT_ROWS 3
#define DEFAULT_GAP 15
#define DEFAULT_BORDER 2
#define DEFAULT_W_CM 4.5f
#define DEFAULT_H_CM 3.5f
#define DEFAULT_PADDING 20
#define DEFAULT_CROP_MARK 20
#define DEFAULT_CROP_THICKNESS 2
#define DEFAULT_BLEED 10
#define DEFAULT_CROP_OFFSET 8

static void print_usage() {
    printf("\nIMGENGINE CLI\n");
    printf("Professional Photo Layout & Print Pipeline\n");
    printf("------------------------------------------------------------\n\n");
    printf("USAGE:\n  imgengine_cli --input <file> [--output <file>] [OPTIONS]\n\n");
    printf("REQUIRED:\n  --input <file>         Input image (jpg/png)\n\n");
    printf("LAYOUT OPTIONS:\n");
    printf("  --cols <int>           Number of columns (default: %d)\n", DEFAULT_COLS);
    printf("  --rows <int>           Number of rows (default: %d)\n", DEFAULT_ROWS);
    printf("  --gap <px>             Gap between photos (default: %d)\n", DEFAULT_GAP);
    printf("  --padding <px>         Page margin (default: %d)\n", DEFAULT_PADDING);
    printf("\nPRINT FEATURES:\n");
    printf("  --bleed <px>           Edge bleed (default: %d)\n", DEFAULT_BLEED);
    printf("  --crop-mark <px>       Mark length (default: %d)\n", DEFAULT_CROP_MARK);
    printf("  --crop-offset <px>     Distance from edge (default: %d)\n", DEFAULT_CROP_OFFSET);

    printf("PHOTO SETTINGS:\n");
    printf("  --width <cm>           Photo width in cm (default: %.1f)\n", DEFAULT_W_CM);
    printf("  --height <cm>          Photo height in cm (default: %.1f)\n", DEFAULT_H_CM);
    printf("  --dpi <int>            Print DPI (default: %d)\n", DEFAULT_DPI);
    printf("  --border <px>          Border thickness (default: %d)\n", DEFAULT_BORDER);
    printf("\n");

    printf("\nEXAMPLES:\n");
    printf(
        "  ./imgengine_cli --input ../input.jpg --output final_output.png --cols 6 --rows 2 --gap \
        15 --width 3.5 --height 3.0 --padding 20 --bleed 10 --crop-mark 25 --crop-offset 8 \n\n");
}

int main(int argc, char **argv) {
    const char *input = NULL;
    const char *output = "output.png";
    int verbose = 1;

    /* Initialize Job with Production Defaults */
    img_job_t job = {.photo_w_cm = DEFAULT_W_CM,
                     .photo_h_cm = DEFAULT_H_CM,
                     .dpi = DEFAULT_DPI,
                     .cols = DEFAULT_COLS,
                     .rows = DEFAULT_ROWS,
                     .gap = DEFAULT_GAP,
                     .border_px = DEFAULT_BORDER,
                     .padding = DEFAULT_PADDING,
                     .crop_mark_px = DEFAULT_CROP_MARK,
                     .crop_thickness = DEFAULT_CROP_THICKNESS,
                     .bleed_px = DEFAULT_BLEED,
                     .crop_offset_px = DEFAULT_CROP_OFFSET,
                     .mode = IMG_FILL,
                     .bg_r = 255,
                     .bg_g = 255,
                     .bg_b = 255};

    if (argc == 1) {
        print_usage();
        return 0;
    }

    /* --- ROBUST ARGUMENT PARSER --- */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc)
            input = argv[++i];
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
            output = argv[++i];
        else if (strcmp(argv[i], "--cols") == 0 && i + 1 < argc)
            job.cols = atoi(argv[++i]);
        else if (strcmp(argv[i], "--rows") == 0 && i + 1 < argc)
            job.rows = atoi(argv[++i]);
        else if (strcmp(argv[i], "--gap") == 0 && i + 1 < argc)
            job.gap = atoi(argv[++i]);
        else if (strcmp(argv[i], "--dpi") == 0 && i + 1 < argc)
            job.dpi = atoi(argv[++i]);
        else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc)
            job.photo_w_cm = atof(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc)
            job.photo_h_cm = atof(argv[++i]);
        else if (strcmp(argv[i], "--border") == 0 && i + 1 < argc)
            job.border_px = atoi(argv[++i]);
        else if (strcmp(argv[i], "--padding") == 0 && i + 1 < argc)
            job.padding = atoi(argv[++i]);
        else if (strcmp(argv[i], "--bleed") == 0 && i + 1 < argc)
            job.bleed_px = atoi(argv[++i]);
        else if (strcmp(argv[i], "--crop-mark") == 0 && i + 1 < argc)
            job.crop_mark_px = atoi(argv[++i]);
        else if (strcmp(argv[i], "--crop-thickness") == 0 && i + 1 < argc)
            job.crop_thickness = atoi(argv[++i]);
        else if (strcmp(argv[i], "--crop-offset") == 0 && i + 1 < argc)
            job.crop_offset_px = atoi(argv[++i]);
        else if (strcmp(argv[i], "--quiet") == 0)
            verbose = 0;
        else if (strcmp(argv[i], "--help") == 0) {
            print_usage();
            return 0;
        } else {
            fprintf(stderr, "❌ Error: Unknown argument '%s'\n", argv[i]);
            return 1;
        }
    }

    if (!input) {
        fprintf(stderr, "❌ Error: --input is mandatory.\n");
        return 1;
    }

    /* --- ENGINE LIFECYCLE --- */
    img_ctx_t *ctx = NULL;
    img_status_t status = imgengine_create(&ctx, POOL_SIZE);

    if (status != IMG_OK) {
        fprintf(stderr, "❌ Critical: Failed to initialize Engine Context.\n");
        return 1;
    }

    if (verbose)
        printf("🚀 Engine Started | Source: %s\n", input);

    status = imgengine_run(ctx, input, output, &job);

    if (status != IMG_OK) {
        fprintf(stderr, "❌ Processing Failed | Status: %d\n", status);
    } else if (verbose) {
        printf("✅ Production Master Generated: %s\n", output);
    }

    imgengine_destroy(ctx);
    return (status == IMG_OK) ? 0 : 1;
}
