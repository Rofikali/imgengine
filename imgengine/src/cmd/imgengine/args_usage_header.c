// ./src/cmd/imgengine/args_usage_header.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_header(const char *bin) {
    (void)bin;
    fprintf(stdout, "\nIMGENGINE CLI\n"
                    "Professional Photo Layout & Print Pipeline\n"
                    "------------------------------------------------------------\n\n");
}
