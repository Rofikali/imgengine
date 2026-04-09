#ifndef IMGENGINE_SPEC_BARRIER_H
#define IMGENGINE_SPEC_BARRIER_H

static inline void img_spec_barrier(void)
{
#if defined(__x86_64__)
    __asm__ __volatile__("lfence" ::: "memory");
#endif
}

#endif