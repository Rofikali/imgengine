// ./src/observability/logger.c

// ./src/observability/logger.c
#define _GNU_SOURCE

#include "observability/logger.h"

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

/*
 * ================= CONFIG =================
 */

#define LOG_RING_SIZE 1024
#define LOG_MSG_SIZE 256

/*
 * ================= FORWARD DECL (🔥 FIX)
 */

static void *logger_loop(void *arg);

/*
 * ================= CELL =================
 */

typedef struct
{
    _Atomic size_t seq;
    char msg[LOG_MSG_SIZE];

} log_cell_t;

/*
 * ================= QUEUE =================
 */

typedef struct
{
    size_t size;
    size_t mask;

    log_cell_t *cells;

    _Atomic size_t head;
    char pad1[64];

    _Atomic size_t tail;
    char pad2[64];

} log_ring_t;

static log_ring_t g_ring;

/*
 * ================= THREAD =================
 */

static pthread_t g_thread;
static volatile int g_running = 0;

/*
 * ================= INIT =================
 */

static inline size_t align_pow2(size_t x)
{
    size_t p = 1;
    while (p < x)
        p <<= 1;
    return p;
}

void img_logger_init(void)
{
    size_t size = align_pow2(LOG_RING_SIZE);

    g_ring.size = size;
    g_ring.mask = size - 1;

    g_ring.cells = aligned_alloc(64, sizeof(log_cell_t) * size);
    if (!g_ring.cells)
        return;

    for (size_t i = 0; i < size; i++)
    {
        atomic_init(&g_ring.cells[i].seq, i);
    }

    atomic_init(&g_ring.head, 0);
    atomic_init(&g_ring.tail, 0);

    g_running = 1;

    /*
     * 🔥 STRICT FUNCTION TYPE (no cast)
     */
    pthread_create(&g_thread, NULL, logger_loop, NULL);
}

/*
 * ================= DESTROY =================
 */

void img_logger_shutdown(void)
{
    g_running = 0;

    pthread_join(g_thread, NULL);

    free(g_ring.cells);
}

/*
 * ================= PUSH =================
 */

static inline int log_ring_push(log_ring_t *q, const char *msg)
{
    log_cell_t *cell;
    size_t pos;

    for (;;)
    {
        pos = atomic_load_explicit(&q->tail, memory_order_relaxed);
        cell = &q->cells[pos & q->mask];

        size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak_explicit(
                    &q->tail, &pos, pos + 1,
                    memory_order_relaxed,
                    memory_order_relaxed))
                break;
        }
        else if (diff < 0)
        {
            return -1; // FULL
        }
    }

    memcpy(cell->msg, msg, LOG_MSG_SIZE);

    atomic_store_explicit(&cell->seq, pos + 1, memory_order_release);

    return 0;
}

/*
 * ================= POP =================
 */

static inline int log_ring_pop(log_ring_t *q, char *out)
{
    log_cell_t *cell;
    size_t pos;

    pos = atomic_load_explicit(&q->head, memory_order_relaxed);
    cell = &q->cells[pos & q->mask];

    size_t seq = atomic_load_explicit(&cell->seq, memory_order_acquire);
    intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

    if (diff < 0)
        return -1;

    if (!atomic_compare_exchange_weak_explicit(
            &q->head, &pos, pos + 1,
            memory_order_relaxed,
            memory_order_relaxed))
        return -1;

    memcpy(out, cell->msg, LOG_MSG_SIZE);

    atomic_store_explicit(
        &cell->seq,
        pos + q->mask + 1,
        memory_order_release);

    return 0;
}

/*
 * ================= LOGGER THREAD =================
 */

static void *logger_loop(void *arg)
{
    (void)arg;

    char buffer[LOG_MSG_SIZE];

    while (__builtin_expect(g_running, 1))
    {
        if (log_ring_pop(&g_ring, buffer) == 0)
        {
            fwrite(buffer, 1, strnlen(buffer, LOG_MSG_SIZE), stdout);
            fputc('\n', stdout);
        }
        else
        {
            __builtin_ia32_pause();
        }
    }

    return NULL;
}

/*
 * ================= PUBLIC API =================
 */

void img_log_write(img_log_level_t level, const char *fmt, ...)
{
    if (__builtin_expect(level < LOG_INFO, 1))
        return;

    char buffer[LOG_MSG_SIZE];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, LOG_MSG_SIZE, fmt, args);
    va_end(args);

    log_ring_push(&g_ring, buffer);
}
