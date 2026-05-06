// ./src/cmd/imgengine/args_usage_options.c
#include "cmd/imgengine/args_internal.h"

void img_cli_print_options_section(void) {
    img_cli_print_input_options();
    img_cli_print_layout_options();
    img_cli_print_photo_options();
    img_cli_print_print_options();
    img_cli_print_runtime_options();
}
