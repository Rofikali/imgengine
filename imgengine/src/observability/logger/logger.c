#include "observability/logger/logger.h"
#include <stdio.h>
#include <stdarg.h>

void img_log_write(img_log_level_t lvl, const char *fmt, ...)
{
    if (lvl < LOG_INFO)
        return;

    va_list args;
    va_start(args, fmt);

    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");

    va_end(args);
}