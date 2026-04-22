// ./src/cmd/imgengine/args_usage_options_print.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_print_options(void)
{
    fprintf(stdout,
            "PRINT FEATURES:\n"
            "  --preset <name>        Template catalog:\n"
            "                         %s - %s\n"
            "                         %s - %s\n"
            "                         %s - %s\n"
            "  --bleed <px>           Edge bleed (default: 10)\n"
            "  --crop-mark <px>       Mark length (default: 20)\n"
            "  --crop-thickness <px>  Mark thickness (default: 2)\n"
            "  --crop-offset <px>     Distance from image edge (default: 8)\n\n",
            img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_45X35),
            img_job_template_description(IMG_JOB_TEMPLATE_PASSPORT_45X35),
            img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_38X35),
            img_job_template_description(IMG_JOB_TEMPLATE_PASSPORT_38X35),
            img_job_template_name(IMG_JOB_TEMPLATE_PRINTREADY_6X6),
            img_job_template_description(IMG_JOB_TEMPLATE_PRINTREADY_6X6));
}
