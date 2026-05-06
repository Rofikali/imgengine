// ./src/cmd/imgengine/args_parse_mode.c
#include "cmd/imgengine/args_internal.h"

#include <stdio.h>
#include <string.h>

int img_cli_parse_mode(const char *text, img_scale_mode_t *mode) {
    if (!text || !mode)
        return -1;

    if (strcmp(text, "fit") == 0) {
        *mode = IMG_FIT;
        return 0;
    }

    if (strcmp(text, "fill") == 0) {
        *mode = IMG_FILL;
        return 0;
    }

    fprintf(stderr, "imgengine: invalid mode '%s' (expected fit|fill)\n", text);
    return -1;
}
