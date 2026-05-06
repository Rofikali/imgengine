// ./src/cmd/imgengine/args_usage_layout.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_usage_section(const char *bin) {
    fprintf(stdout,
            "USAGE:\n"
            "  %s --input <file> [--output <file>] [OPTIONS]\n\n"
            "REQUIRED:\n"
            "  --input <file>         Input image or raw frame\n\n"
            "OUTPUT:\n"
            "  --output <file>        Output file .jpg/.png/.pdf (default: output.jpg)\n\n",
            bin);
}
