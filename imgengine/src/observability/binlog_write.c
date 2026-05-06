// ./src/observability/binlog_write.c
#define _GNU_SOURCE

#include "observability/binlog.h"
#include "core/time.h"

extern img_binlog_t g_binlog;
extern uint32_t img_binlog_tls_cpu_id(void);

/*
 * img_binlog_write()
 *
 * Lock-free, atomic-free ring buffer write.
 * SPSC per CPU — only the pinned worker thread writes to its slot.
 * Consumer (flush thread) reads head. No CAS needed.
 *
 * Drop policy: if ring is full, drop the entry silently.
 * Never blocks. Never allocates.
 */
void img_binlog_write(img_binlog_t *log, uint32_t event, uint64_t arg0, uint64_t arg1,
                      uint64_t arg2) {
    uint32_t cpu = img_binlog_tls_cpu_id();

    if (__builtin_expect(cpu >= log->cpu_count, 0))
        return;

    img_binlog_cpu_t *c = &log->cpu_logs[cpu];

    uint32_t tail = c->tail;
    uint32_t next = (tail + 1) & c->mask;

    if (__builtin_expect(next == c->head, 0))
        return;

    img_log_entry_t *e = &c->entries[tail];

    e->timestamp = img_now_ns();
    e->event_id = event;
    e->cpu = cpu;
    e->arg0 = arg0;
    e->arg1 = arg1;
    e->arg2 = arg2;

    c->tail = next;
}
