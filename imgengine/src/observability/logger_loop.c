// ./src/observability/logger_loop.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"

#include <stdio.h>
#include <string.h>

void *img_logger_loop(void *arg) {
    (void)arg;

    char buffer[LOG_MSG_SIZE];

    while (__builtin_expect(g_logger_running, 1)) {
        if (log_ring_pop(&g_ring, buffer) == 0) {
            fwrite(buffer, 1, strnlen(buffer, LOG_MSG_SIZE), stdout);
            fputc('\n', stdout);
        } else {
            __builtin_ia32_pause();
        }
    }

    return NULL;
}
