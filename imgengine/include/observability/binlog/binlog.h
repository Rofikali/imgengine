#ifndef IMGENGINE_BINLOG_H
#define IMGENGINE_BINLOG_H

#include <stdint.h>

#define IMG_MAX_CPUS 128

typedef struct
{
    uint64_t ts;
    uint32_t event;
    uint32_t cpu;

    uint64_t a0, a1, a2;

} img_log_entry_t;

typedef struct
{
    img_log_entry_t *entries;

    uint32_t size;
    uint32_t mask;

    uint32_t head;
    uint32_t tail;

} img_binlog_cpu_t;

typedef struct
{
    uint32_t cpu_count;
    img_binlog_cpu_t cpus[IMG_MAX_CPUS];

} img_binlog_t;

int img_binlog_init(img_binlog_t *log, uint32_t cpus, uint32_t power);
void img_binlog_destroy(img_binlog_t *log);

void img_binlog_write(
    img_binlog_t *log,
    uint32_t event,
    uint64_t a0,
    uint64_t a1,
    uint64_t a2);

#endif