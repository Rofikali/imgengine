// ./include/cmd/imgengine/args.h

#ifndef IMGENGINE_CLI_ARGS_H
#define IMGENGINE_CLI_ARGS_H

#include <stdint.h>
#include <stdbool.h>
#include "pipeline/job.h"

typedef enum {
    IMG_CLI_INPUT_FORMAT_ENCODED = 0,
    IMG_CLI_INPUT_FORMAT_RAW_RGB24 = 1,
} img_cli_input_format_t;

typedef struct {
    const char *input_path;
    const char *output_path;
    img_cli_input_format_t input_format;

    /* raw ingress */
    uint32_t input_width;
    bool has_input_width;
    uint32_t input_height;
    bool has_input_height;
    uint32_t input_stride;
    bool has_input_stride;

    /* layout */
    uint32_t cols;
    bool has_cols;
    uint32_t rows;
    bool has_rows;
    uint32_t gap;
    bool has_gap;
    uint32_t padding;
    bool has_padding;
    img_job_template_t preset_template;
    bool has_preset;

    /* photo */
    float photo_w_cm;
    bool has_photo_w_cm;
    float photo_h_cm;
    bool has_photo_h_cm;
    uint32_t dpi;
    bool has_dpi;
    uint32_t border;
    bool has_border;

    /* print */
    uint32_t bleed;
    bool has_bleed;
    uint32_t crop_mark;
    bool has_crop_mark;
    uint32_t crop_thickness;
    bool has_crop_thickness;
    uint32_t crop_offset;
    bool has_crop_offset;

    /* runtime */
    uint32_t threads;
    bool has_threads;

    img_scale_mode_t mode;
    bool mode_set;

    bool verbose;
    bool quiet;
    bool help;

} img_cli_options_t;

int img_parse_args(int argc, char **argv, img_cli_options_t *opts);
void img_print_usage(const char *bin);

#endif
