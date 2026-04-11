// ./src/observability/binlog.c

#include "observability/binlog.h"
#include "core/time.h"

#include <stdlib.h>
#include <string.h>

img_binlog_t g_binlog;

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
