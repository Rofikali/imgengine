// include/observability/binlog_fast.h

#ifndef IMGENGINE_BINLOG_FAST_H
#define IMGENGINE_BINLOG_FAST_H

#include "observability/binlog.h"
#include "observability/events.h"

// global instance
extern img_binlog_t g_binlog;

/*
 * 🔥 GENERIC LOG (BASE PRIMITIVE)
 */
#define IMG_LOG(event, a0, a1, a2)        \
    do                                    \
    {                                     \
        img_binlog_write(&g_binlog,       \
                         (event),         \
                         (uint64_t)(a0),  \
                         (uint64_t)(a1),  \
                         (uint64_t)(a2)); \
    } while (0)

/*
 * 🔥 SPECIALIZED MACROS
 * (define ONLY ONCE — no duplication)
 */
#define IMG_LOG_LATENCY(cycles, count, extra) \
    IMG_LOG(IMG_EVENT_LATENCY, cycles, count, extra)

#define IMG_LOG_ERROR(code) \
    IMG_LOG(IMG_EVENT_ERROR, code, 0, 0)

#endif
