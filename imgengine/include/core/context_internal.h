/* include/core/context_internal.h */

#ifndef IMGENGINE_CONTEXT_INTERNAL_H
#define IMGENGINE_CONTEXT_INTERNAL_H

#include <stdint.h>
#include "arch/cpu_caps.h"
// #include "runtime/worker.h"
typedef struct img_worker img_worker_t;

/*
 * Forward declarations
 */
typedef struct img_slab_pool img_slab_pool_t;
typedef struct img_arena img_arena_t;
// typedef struct cpu_caps cpu_caps_t;

/*
 * ================= ENGINE =================
 */
struct img_engine // ✅ FIXED NAME
{
    uint32_t worker_count;
    // struct img_worker_s *workers;

    img_worker_t *workers;

    cpu_caps_t caps;

    img_slab_pool_t *global_pool;

    void *user_data;
};

typedef struct img_engine img_engine_t;

/*
 * ================= CONTEXT =================
 */

typedef struct __attribute__((aligned(64))) img_ctx
{
    uint32_t thread_id;

    img_slab_pool_t *local_pool;
    img_arena_t *scratch_arena;

    cpu_caps_t caps;

    void *op_params;    // 🔥 REQUIRED
    void *fused_params; // 🔥 for fused execution

} img_ctx_t;

// typedef struct __attribute__((aligned(64))) img_ctx
// {
//     uint32_t thread_id;

//     img_slab_pool_t *local_pool;
//     img_arena_t *scratch_arena;

//     cpu_caps_t caps;

//     /*
//      * 🔥 FUSED PARAM BLOCK (HOT, CACHE-RESIDENT)
//      */
//     void *fused_params;

// } img_ctx_t;

#endif
