/* runtime/backpressure.h */
#ifndef IMGENGINE_RUNTIME_BACKPRESSURE_H
#define IMGENGINE_RUNTIME_BACKPRESSURE_H

#include <stdbool.h>
#include "runtime/queue_spsc.h"

/**
 * @brief Returns true if queue is overloaded
 */
bool img_backpressure_check(img_queue_t *q);

#endif