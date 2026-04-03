/* runtime/affinity.c */

#define _GNU_SOURCE
#include "runtime/affinity.h"
#include <pthread.h>
#include <sched.h>

void img_pin_thread_to_core(uint32_t cpu_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    pthread_setaffinity_np(pthread_self(),
                           sizeof(cpu_set_t),
                           &cpuset);
}