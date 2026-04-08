#ifndef IMGENGINE_BINLOG_FAST_H
#define IMGENGINE_BINLOG_FAST_H

#include "observability/binlog/binlog.h"
#include "observability/events/events.h"

extern img_binlog_t g_binlog;

#define IMG_LOG(ev, a0, a1, a2) \
    img_binlog_write(&g_binlog, ev, (uint64_t)a0, (uint64_t)a1, (uint64_t)a2)

#define IMG_LOG_LATENCY(cycles, ops, extra) \
    IMG_LOG(IMG_EVENT_LATENCY, cycles, ops, extra)

#define IMG_LOG_ERROR(code) \
    IMG_LOG(IMG_EVENT_ERROR, code, 0, 0)

#endif