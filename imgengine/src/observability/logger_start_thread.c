// ./src/observability/logger_start_thread.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"

int img_logger_start_thread(void) {
    g_logger_running = 1;
    if (pthread_create(&g_logger_thread, NULL, img_logger_loop, NULL) != 0) {
        g_logger_running = 0;
        return -1;
    }

    g_logger_thread_started = 1;
    return 0;
}
