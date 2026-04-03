/* runtime/affinity.h */

#ifndef IMGENGINE_RUNTIME_AFFINITY_H
#define IMGENGINE_RUNTIME_AFFINITY_H

#include <stdint.h>
#include <pthread.h>

/**
 * Pin current thread to CPU core
 */
void img_pin_thread_to_core(uint32_t cpu_id);

/**
 * Pin thread to CPU core
 */

void img_set_thread_affinity(pthread_t thread, int cpu_id);

#endif
