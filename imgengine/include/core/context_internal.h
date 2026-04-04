/* include/core/context_internal.h */

#ifndef IMGENGINE_CONTEXT_INTERNAL_H
#define IMGENGINE_CONTEXT_INTERNAL_H

#include <stdint.h>
#include "arch/cpu_caps.h"

/*
 * Forward declarations
 */
typedef struct img_slab_pool img_slab_pool_t;
typedef struct img_arena img_arena_t;

/*
 * ================= ENGINE =================
 */
struct img_engine // ✅ FIXED NAME
{
    uint32_t worker_count;
    struct img_worker_s *workers;

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

} img_ctx_t;

#endif

// #ifndef IMGENGINE_CONTEXT_INTERNAL_H
// #define IMGENGINE_CONTEXT_INTERNAL_H

// #include <stdint.h>
// #include <stddef.h>

// #include "arch/cpu_caps.h"

// /*
//  * 🔥 Forward declarations (NO heavy includes)
//  */
// typedef struct img_slab_pool img_slab_pool_t;
// typedef struct img_arena img_arena_t;

// // Forward declare worker
// struct img_worker_t;

// /*
//  * ================= ENGINE =================
//  * (COLD + SEMI-HOT DATA)
//  */
// struct img_engine_t
// {
//     // 🔥 HOT-ish
//     uint32_t worker_count;
//     struct img_worker_t *workers;

//     // 🔥 SEMI-HOT
//     cpu_caps_t caps;

//     // ❄️ COLD
//     img_slab_pool_t *global_pool;

//     // EXTENSION
//     void *user_data;
// };

// typedef struct img_engine img_engine_t;

// /*
//  * ================= THREAD CONTEXT =================
//  * (🔥 HOT PATH STRUCT — CACHE ALIGNED)
//  */
// typedef struct __attribute__((aligned(64))) img_ctx
// {
//     uint32_t thread_id;

//     // 🔥 HOT
//     img_slab_pool_t *local_pool;

//     // 🔥 SCRATCH (TEMP OPS)
//     img_arena_t *scratch_arena;

//     // 🔥 CPU FEATURES (cached)
//     cpu_caps_t caps;

// } img_ctx_t;

// /*
//  * Lifecycle
//  */
// img_ctx_t *img_ctx_create(uint32_t thread_id, cpu_caps_t caps);
// void img_ctx_destroy(img_ctx_t *ctx);

// #endif