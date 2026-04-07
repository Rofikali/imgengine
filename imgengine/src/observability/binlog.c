// src/observability/binlog.c

#include "observability/binlog.h"
#include "core/time.h"

#include <stdlib.h>
#include <string.h>

/*
 * 🔥 TLS CPU ID (NO SYSCALL)
 */
static __thread uint32_t tls_cpu_id = 0;

void img_binlog_set_cpu(uint32_t cpu)
{
    tls_cpu_id = cpu;
}

/*
 * 🔥 INIT
 */
int img_binlog_init(
    img_binlog_t *log,
    uint32_t cpu_count,
    uint32_t power)
{
    if (!log || cpu_count == 0 || cpu_count > IMG_MAX_CPUS)
        return -1;

    memset(log, 0, sizeof(*log));

    log->cpu_count = cpu_count;

    uint32_t size = 1u << power;

    for (uint32_t i = 0; i < cpu_count; i++)
    {
        img_binlog_cpu_t *c = &log->cpu_logs[i];

        c->entries = aligned_alloc(
            64,
            sizeof(img_log_entry_t) * size);

        if (!c->entries)
            return -1;

        c->size = size;
        c->mask = size - 1;

        c->head = 0;
        c->tail = 0;
    }

    return 0;
}

/*
 * 🔥 DESTROY
 */
void img_binlog_destroy(img_binlog_t *log)
{
    if (!log)
        return;

    for (uint32_t i = 0; i < log->cpu_count; i++)
    {
        free(log->cpu_logs[i].entries);
    }
}

/*
 * 🔥 WRITE (ZERO LOCK, ZERO ATOMIC)
 */
void img_binlog_write(
    img_binlog_t *log,
    uint32_t event,
    uint64_t arg0,
    uint64_t arg1,
    uint64_t arg2)
{
    uint32_t cpu = tls_cpu_id;

    if (cpu >= log->cpu_count)
        return;

    img_binlog_cpu_t *c = &log->cpu_logs[cpu];

    uint32_t tail = c->tail;
    uint32_t next = (tail + 1) & c->mask;

    if (next == c->head)
        return; // drop (no blocking ever)

    img_log_entry_t *e = &c->entries[tail];

    e->timestamp = img_now_ns();
    e->event_id = event;
    e->cpu = cpu;

    e->arg0 = arg0;
    e->arg1 = arg1;
    e->arg2 = arg2;

    c->tail = next;
}

// #define _GNU_SOURCE

// #include "observability/binlog.h"

// #include <stdlib.h>
// #include <sched.h>
// #include <unistd.h>

// #include "core/time.h"

// /*
//  * 🔥 L10 CPU CACHE (TLS)
//  *
//  * - One-time init per thread
//  * - No syscall in hot path
//  * - No contention
//  */

// static __thread uint32_t tls_cpu = 0;
// static __thread int tls_cpu_initialized = 0;

// /*
//  * 🔥 SLOW PATH (ONLY ONCE PER THREAD)
//  */
// static inline uint32_t detect_cpu_initial(void)
// {
// #if defined(__linux__) && defined(__GLIBC__)
//     int cpu = sched_getcpu();
//     if (cpu >= 0)
//         return (uint32_t)cpu;
// #endif

//     return 0;
// }

// /*
//  * 🔥 FAST PATH (ZERO COST AFTER INIT)
//  */
// static inline uint32_t get_cpu(void)
// {
//     if (__builtin_expect(tls_cpu_initialized, 1))
//         return tls_cpu;

//     // 🔥 one-time init
//     tls_cpu = detect_cpu_initial();
//     tls_cpu_initialized = 1;

//     return tls_cpu;
// }

// // ================================
// // INIT
// // ================================

// int img_binlog_init(img_binlog_t *log, uint32_t power)
// {
//     if (!log)
//         return -1;

//     uint32_t size = 1u << power;

//     log->entries = aligned_alloc(64, sizeof(img_log_entry_t) * size);
//     if (!log->entries)
//         return -1;

//     log->size = size;
//     log->mask = size - 1;

//     atomic_init(&log->head, 0);
//     atomic_init(&log->tail, 0);

//     return 0;
// }

// // ================================
// // DESTROY
// // ================================

// void img_binlog_destroy(img_binlog_t *log)
// {
//     if (!log)
//         return;

//     free(log->entries);
// }

// // ================================
// // WRITE (LOCK-FREE HOT PATH)
// // ================================

// int img_binlog_write(
//     img_binlog_t *log,
//     uint32_t event,
//     uint64_t arg0,
//     uint64_t arg1,
//     uint64_t arg2)
// {
//     uint32_t tail = atomic_load_explicit(&log->tail, memory_order_relaxed);
//     uint32_t next = (tail + 1) & log->mask;

//     uint32_t head = atomic_load_explicit(&log->head, memory_order_acquire);

//     if (__builtin_expect(next == head, 0))
//         return -1; // full → drop

//     img_log_entry_t *e = &log->entries[tail];

//     e->timestamp = img_now_ns();
//     e->event_id = event;
//     e->cpu = get_cpu(); // 🔥 ZERO-COST
//     e->arg0 = arg0;
//     e->arg1 = arg1;
//     e->arg2 = arg2;

//     atomic_store_explicit(&log->tail, next, memory_order_release);

//     return 0;
// }
