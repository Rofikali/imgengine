// src/api/api.c

#include "api/v1/img_api.h"
#include "core/context_internal.h"
#include "runtime/worker.h"
#include "memory/slab.h"
#include "arch/cpu_caps.h"
#include "pipeline/jump_table.h"
#include "pipeline/hardware_registry.h"

#include <string.h>

// 🔥 STATIC ENGINE
static img_engine_t g_engine;
static img_worker_t g_workers[64];

img_engine_t *img_api_init(uint32_t workers)
{
    if (workers == 0 || workers > 64)
        return NULL;

    memset(&g_engine, 0, sizeof(g_engine));

    g_engine.worker_count = workers;
    g_engine.workers = g_workers;

    // 🔥 CPU DETECT
    g_engine.caps = img_cpu_detect_caps();

    // 🔥 GLOBAL POOL (boundary alloc OK)
    g_engine.global_pool = img_slab_create(
        64 * 1024 * 1024,
        64 * 1024);

    // 🔥 INIT DISPATCH
    img_jump_table_init(g_engine.caps);
    img_hw_register_kernels(g_engine.caps);

    // 🔥 INIT WORKERS
    for (uint32_t i = 0; i < workers; i++)
    {
        img_worker_init(&g_workers[i], i);
    }

    return &g_engine;
}

void img_api_shutdown(img_engine_t *engine)
{
    if (!engine)
        return;

    for (uint32_t i = 0; i < engine->worker_count; i++)
    {
        img_worker_stop(&engine->workers[i]);
        img_worker_join(&engine->workers[i]);
    }

    img_slab_destroy(engine->global_pool);
}

// #include "api/v1/img_api.h"
// #include "core/context_internal.h"
// #include "memory/slab.h"
// #include "memory/arena.h"
// #include "arch/cpu_caps.h"
// #include "pipeline/jump_table.h"
// #include "pipeline/hardware_registry.h"

// #include <string.h>

// // 🔥 STATIC ENGINE (NO MALLOC)
// static img_engine_t g_engine;
// static img_worker_t g_workers[64]; // max limit (compile-time)

// // ==========================================
// // INIT (NO MALLOC)
// // ==========================================
// img_engine_t *img_api_init(uint32_t workers)
// {
//     if (workers == 0 || workers > 64)
//         return NULL;

//     memset(&g_engine, 0, sizeof(g_engine));

//     g_engine.worker_count = workers;
//     g_engine.workers = g_workers;

//     // 🔥 CPU DETECT
//     g_engine.caps = img_cpu_detect_caps();

//     // 🔥 GLOBAL SLAB (still malloc internally → OK: system boundary)
//     g_engine.global_pool = img_slab_create(
//         64 * 1024 * 1024,
//         64 * 1024);

//     // 🔥 JUMP TABLE INIT
//     img_jump_table_init(g_engine.caps);
//     img_hw_register_kernels(g_engine.caps);

//     // 🔥 WORKERS INIT (NO malloc here)
//     for (uint32_t i = 0; i < workers; i++)
//     {
//         img_worker_init(&g_engine.workers[i], i);
//     }

//     return &g_engine;
// }

// // ==========================================
// // SHUTDOWN
// // ==========================================
// void img_api_shutdown(img_engine_t *engine)
// {
//     if (!engine)
//         return;

//     for (uint32_t i = 0; i < engine->worker_count; i++)
//     {
//         engine->workers[i].running = 0;
//     }

//     img_slab_destroy(engine->global_pool);
// }
