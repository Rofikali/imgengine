// ./src/cmd/imgengine/args_usage_options_layout.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_layout_options(void) {
    fprintf(stdout, "LAYOUT OPTIONS:\n"
                    "  --cols <int>           Columns (default: 2)\n"
                    "  --rows <int>           Rows    (default: 3)\n"
                    "  --gap <px>             Gap between photos (default: 15)\n"
                    "  --padding <px>         Page margin (default: 20)\n\n");
}
