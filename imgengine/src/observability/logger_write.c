// ./src/observability/logger_write.c
#include "observability/logger.h"
#include "observability/logger_internal.h"

#include <stdarg.h>
#include <stdio.h>

void img_log_write(img_log_level_t level, const char *fmt, ...) {
    if (__builtin_expect(level < LOG_INFO, 1))
        return;

    char buffer[LOG_MSG_SIZE];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, LOG_MSG_SIZE, fmt, args);
    va_end(args);

    log_ring_push(&g_ring, buffer);
}
