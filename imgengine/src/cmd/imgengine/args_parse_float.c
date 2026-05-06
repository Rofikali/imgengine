// ./src/cmd/imgengine/args_parse_float.c
#include "cmd/imgengine/args_internal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int img_cli_parse_float(const char *text, float *out, const char *name) {
    char *end = NULL;
    errno = 0;
    if (!text || *text == '\0' || text[0] == '-') {
        fprintf(stderr, "imgengine: invalid %s value '%s'\n", name, text ? text : "(null)");
        return -1;
    }

    float value = strtof(text, &end);
    if (errno != 0 || !end || *end != '\0') {
        fprintf(stderr, "imgengine: invalid %s value '%s'\n", name, text ? text : "(null)");
        return -1;
    }

    *out = value;
    return 0;
}
