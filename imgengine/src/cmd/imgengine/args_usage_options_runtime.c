// ./src/cmd/imgengine/args_usage_options_runtime.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_runtime_options(void) {
    fprintf(stdout, "OTHER:\n"
                    "  --threads <int>        Worker threads (default: 1)\n"
                    "  --mode <fit|fill>      Scaling mode (default: fill)\n"
                    "  --quiet                Suppress informational output\n"
                    "  --help                 Show this help\n\n");
}
