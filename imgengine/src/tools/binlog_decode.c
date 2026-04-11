// ./src/tools/binlog_decode.c

// ./src/tools/binlog_decode.c



#include <stdio.h>
#include "observability/binlog.h"
#include "observability/events.h"

void decode(img_log_entry_t *e)
{
    switch (e->event_id)
    {
    case IMG_EVENT_LATENCY:
        printf("[LAT] %lu cycles count=%lu extra=%lu\n",
               e->arg0, e->arg1, e->arg2);
        break;

    case IMG_EVENT_ERROR:
        printf("[ERR] code=%lu\n", e->arg0);
        break;

    default:
        printf("[UNK] event=%u\n", e->event_id);
        break;
    }
}