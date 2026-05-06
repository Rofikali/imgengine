// ./src/cmd/imgengine/args_usage_options_input.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_input_options(void) {
    fprintf(stdout,
            "INPUT:\n"
            "  --input <path>              Source image or raw frame path\n"
            "  --output <path>             Output image or PDF path\n"
            "  --input-format <kind>       encoded | raw-rgb24 (default: encoded)\n"
            "  --input-width <px>          Required for raw-rgb24\n"
            "  --input-height <px>         Required for raw-rgb24\n"
            "  --input-stride <bytes>      Optional raw row stride (default: width * 3)\n\n");
}
