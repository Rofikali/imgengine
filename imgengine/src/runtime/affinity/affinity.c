#include "runtime/affinity/affinity.h"
#include <pthread.h>
#include <sched.h>

void img_pin_thread_to_core(uint32_t cpu)
{
#ifdef __linux__
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);

    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
#else
    (void)cpu;
#endif
}