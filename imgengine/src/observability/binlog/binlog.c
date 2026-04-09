#include "observability/binlog/binlog.h"
#include "core/time.h"

#include <stdlib.h>
#include <string.h>

static __thread uint32_t tls_cpu;

void img_binlog_set_cpu(uint32_t id)
{
    tls_cpu = id;
}

int img_binlog_init(img_binlog_t *log, uint32_t cpus, uint32_t power)
{
    if (!log || cpus == 0 || cpus > IMG_MAX_CPUS)
        return -1;

    memset(log, 0, sizeof(*log));
    log->cpu_count = cpus;

    uint32_t size = 1u << power;

    for (uint32_t i = 0; i < cpus; i++)
    {
        img_binlog_cpu_t *c = &log->cpus[i];

        c->entries = aligned_alloc(
            64,
            sizeof(img_log_entry_t) * size);

        if (!c->entries)
            return -1;

        c->size = size;
        c->mask = size - 1;
    }

    return 0;
}

void img_binlog_write(
    img_binlog_t *log,
    uint32_t event,
    uint64_t a0,
    uint64_t a1,
    uint64_t a2)
{
    uint32_t cpu = tls_cpu;

    if (cpu >= log->cpu_count)
        return;

    img_binlog_cpu_t *c = &log->cpus[cpu];

    uint32_t t = c->tail;
    uint32_t next = (t + 1) & c->mask;

    if (next == c->head)
        return; // drop (never block)

    img_log_entry_t *e = &c->entries[t];

    e->ts = img_now_ns();
    e->event = event;
    e->cpu = cpu;

    e->a0 = a0;
    e->a1 = a1;
    e->a2 = a2;

    c->tail = next;
}