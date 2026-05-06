// ./src/cmd/imgengine/args_parse_u32.c
#include "cmd/imgengine/args_internal.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int img_cli_parse_u32(const char *text, uint32_t *out, const char *name) {
    char *end = NULL;
    errno = 0;
    if (!text || *text == '\0' || text[0] == '-') {
        fprintf(stderr, "imgengine: invalid %s value '%s'\n", name, text ? text : "(null)");
        return -1;
    }

    unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || !end || *end != '\0' || value > UINT32_MAX) {
        fprintf(stderr, "imgengine: invalid %s value '%s'\n", name, text ? text : "(null)");
        return -1;
    }

    *out = (uint32_t)value;
    return 0;
}
