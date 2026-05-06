// ./src/observability/logger_stop_thread.c
#define _GNU_SOURCE

#include "observability/logger_internal.h"

void img_logger_stop_thread(void) {
    g_logger_running = 0;
    if (g_logger_thread_started) {
        pthread_join(g_logger_thread, NULL);
        g_logger_thread_started = 0;
    }
}
