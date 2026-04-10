
// observability/events.h 

#ifndef IMGENGINE_EVENTS_H
#define IMGENGINE_EVENTS_H

#include <stdint.h>

/*
 * 🔥 STABLE EVENT IDS (ABI CONTRACT)
 */
typedef enum
{
    IMG_EVENT_LATENCY = 1,
    IMG_EVENT_ERROR = 2,
    IMG_EVENT_BATCH = 3,

} img_event_id_t;

#endif