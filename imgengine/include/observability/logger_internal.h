// ./include/observability/logger_internal.h
#ifndef IMGENGINE_OBSERVABILITY_LOGGER_INTERNAL_H
#define IMGENGINE_OBSERVABILITY_LOGGER_INTERNAL_H

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#define LOG_RING_SIZE 1024
#define LOG_MSG_SIZE 256

typedef struct {
    _Atomic size_t seq;
    char msg[LOG_MSG_SIZE];
} log_cell_t;

typedef struct {
    size_t size;
    size_t mask;

    log_cell_t *cells;

    _Atomic size_t head;
    char pad1[64];

    _Atomic size_t tail;
    char pad2[64];
} log_ring_t;

extern log_ring_t g_ring;
extern pthread_t g_logger_thread;
extern volatile int g_logger_running;
extern int g_logger_thread_started;

int log_ring_push(log_ring_t *q, const char *msg);
int log_ring_pop(log_ring_t *q, char *out);
void *img_logger_loop(void *arg);
int img_logger_init_ring(void);
int img_logger_start_thread(void);
void img_logger_clear_ring(void);
void img_logger_stop_thread(void);

#endif /* IMGENGINE_OBSERVABILITY_LOGGER_INTERNAL_H */
