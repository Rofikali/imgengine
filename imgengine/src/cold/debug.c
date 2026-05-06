// ./src/cold/debug.c

// src/cold/debug.c

#include "cold/debug.h"
#include "core/context_internal.h"

#include <stdio.h>
#include <stdatomic.h>

// 🔥 global debug level (cold path only)
static _Atomic int g_debug_level = IMG_DEBUG_NONE;

void img_debug_set_level(img_debug_level_t lvl) { atomic_store(&g_debug_level, lvl); }

#define DEBUG_ENABLED(lvl) (atomic_load(&g_debug_level) >= (lvl))

void img_debug_dump_ctx(const img_ctx_t *ctx) {
    if (!ctx || !DEBUG_ENABLED(IMG_DEBUG_BASIC))
        return;

    printf("[CTX] thread=%u slab=%p caps=0x%x\n", ctx->thread_id, (void *)ctx->local_pool,
           (unsigned int)ctx->caps);
}

void img_debug_dump_slab_usage(const img_ctx_t *ctx) {
    if (!ctx || !DEBUG_ENABLED(IMG_DEBUG_VERBOSE))
        return;

    printf("[SLAB] pool=%p\n", (void *)ctx->local_pool);
}

void img_debug_print_cpu_info(void) {
    if (!DEBUG_ENABLED(IMG_DEBUG_BASIC))
        return;

#if defined(__x86_64__)
    printf("[CPU] x86_64 detected\n");
#elif defined(__aarch64__)
    printf("[CPU] ARM64 detected\n");
#endif
}