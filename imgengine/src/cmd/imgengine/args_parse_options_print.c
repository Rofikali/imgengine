// ./src/cmd/imgengine/args_parse_options_print.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

static int img_cli_parse_preset(const char *optarg, img_cli_options_t *opts)
{
    img_job_template_t template_id;

    if (!optarg || !opts)
        return -1;

    if (img_job_template_lookup(optarg, &template_id) == 0)
    {
        opts->preset_template = template_id;
        opts->has_preset = true;
        return 0;
    }

    fprintf(stderr,
            "imgengine: unknown preset '%s' (use %s, %s, %s)\n",
            optarg,
            img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_45X35),
            img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_38X35),
            img_job_template_name(IMG_JOB_TEMPLATE_PRINTREADY_6X6));
    return -1;
}

int img_cli_parse_options_print(int opt, const char *optarg, img_cli_options_t *opts)
{
    switch (opt)
    {
    case 14:
        return img_cli_parse_preset(optarg, opts);
    case 9:
        if (img_cli_parse_u32(optarg, &opts->bleed, "bleed") != 0)
            return -1;
        opts->has_bleed = true;
        return 0;
    case 10:
        if (img_cli_parse_u32(optarg, &opts->crop_mark, "crop-mark") != 0)
            return -1;
        opts->has_crop_mark = true;
        return 0;
    case 11:
        if (img_cli_parse_u32(optarg, &opts->crop_thickness, "crop-thickness") != 0)
            return -1;
        opts->has_crop_thickness = true;
        return 0;
    case 12:
        if (img_cli_parse_u32(optarg, &opts->crop_offset, "crop-offset") != 0)
            return -1;
        opts->has_crop_offset = true;
        return 0;
    case 't':
        if (img_cli_parse_u32(optarg, &opts->threads, "threads") != 0)
            return -1;
        if (opts->threads == 0)
        {
            fprintf(stderr, "imgengine: threads must be at least 1\n");
            return -1;
        }
        if (opts->threads > 64)
        {
            fprintf(stderr, "imgengine: threads must be <= 64\n");
            return -1;
        }
        opts->has_threads = true;
        return 0;
    case 13:
        if (img_cli_parse_mode(optarg, &opts->mode) != 0)
            return -1;
        opts->mode_set = true;
        return 0;
    default:
        return -1;
    }
}
