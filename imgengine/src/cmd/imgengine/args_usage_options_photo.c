// ./src/cmd/imgengine/args_usage_options_photo.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>

void img_cli_print_photo_options(void) {
    fprintf(stdout, "PHOTO SETTINGS:\n"
                    "  --width <cm>           Photo width  in cm (default: 4.5)\n"
                    "  --height <cm>          Photo height in cm (default: 3.5)\n"
                    "  --dpi <int>            Print DPI (default: 300)\n"
                    "  --border <px>          Border thickness (default: 2)\n\n");
}
