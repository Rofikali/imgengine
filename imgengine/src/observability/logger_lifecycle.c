// ./src/observability/logger_lifecycle.c
#define _GNU_SOURCE

#include "observability/logger.h"
#include "observability/logger_internal.h"

void img_logger_init(void) {
    if (img_logger_init_ring() != 0)
        return;

    if (img_logger_start_thread() != 0)
        img_logger_clear_ring();
}

void img_logger_shutdown(void) {
    img_logger_stop_thread();
    img_logger_clear_ring();
}
