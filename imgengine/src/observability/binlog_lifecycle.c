// ./src/observability/binlog_lifecycle.c
#define _GNU_SOURCE

#include "observability/binlog.h"

#include <stdlib.h>
#include <string.h>

int img_binlog_init(img_binlog_t *log, uint32_t cpu_count, uint32_t power) {
    if (!log || cpu_count == 0 || cpu_count > IMG_MAX_CPUS)
        return -1;

    memset(log, 0, sizeof(*log));

    log->cpu_count = cpu_count;

    uint32_t size = 1u << power;

    for (uint32_t i = 0; i < cpu_count; i++) {
        img_binlog_cpu_t *c = &log->cpu_logs[i];

        c->entries = aligned_alloc(64, sizeof(img_log_entry_t) * size);

        if (!c->entries)
            return -1;

        c->size = size;
        c->mask = size - 1;
        c->head = 0;
        c->tail = 0;
    }

    return 0;
}

void img_binlog_destroy(img_binlog_t *log) {
    if (!log)
        return;

    for (uint32_t i = 0; i < log->cpu_count; i++)
        free(log->cpu_logs[i].entries);
}
