#ifndef IMGENGINE_EVENTS_H
#define IMGENGINE_EVENTS_H

typedef enum
{
    IMG_EVENT_LATENCY = 1,
    IMG_EVENT_ERROR   = 2,
    IMG_EVENT_BATCH   = 3,
    IMG_EVENT_IO      = 4,
    IMG_EVENT_SCHED   = 5,

} img_event_id_t;

#endif