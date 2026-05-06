// ./src/cmd/imgengine/args_usage_examples.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_examples_section(const char *bin) {
    fprintf(stdout,
            "EXAMPLES:\n\n"
            "  Passport sheet:\n"
            "    %s --input photo.jpg --cols 3 --rows 2\n\n"
            "  Print-ready A4 with crop marks:\n"
            "    %s --input photo.jpg --output sheet.jpg \\\n"
            "        --cols 6 --rows 2 --width 3.5 --height 3.0 \\\n"
            "        --bleed 10 --crop-mark 25 --crop-offset 8\n\n"
            "  PDF export:\n"
            "    %s --input photo.jpg --output sheet.pdf --dpi 300\n\n"
            "  Raw RGB24 ingress:\n"
            "    %s --input frame.rgb --input-format raw-rgb24 \\\n"
            "        --input-width 4096 --input-height 4096 --preset passport-45x35\n\n",
            bin, bin, bin, bin);
}
