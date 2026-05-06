// ./src/observability/logger.c

// Thin coordination translation unit.
// Concrete logger work now lives in:
//   - src/observability/logger_ring.c
//   - src/observability/logger_lifecycle.c
//   - src/observability/logger_write.c

#include "observability/logger_internal.h"

pthread_t g_logger_thread;
volatile int g_logger_running = 0;
int g_logger_thread_started = 0;
