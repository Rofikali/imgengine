// ./src/cmd/imgengine/args_usage.c
#include "cmd/imgengine/args_internal.h"

void img_print_usage(const char *bin) {
    img_cli_print_header(bin);
    img_cli_print_usage_section(bin);
    img_cli_print_options_section();
    img_cli_print_examples_section(bin);
}
