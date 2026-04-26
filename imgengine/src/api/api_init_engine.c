// ./src/api/api_init_engine.c
#include "api/api_init_internal.h"
#include "memory/slab.h"
#include "runtime/template_registry.h"
#include "startup/engine_init_internal.h"
#include "pipeline/generated.h"

int img_api_init_prepare_engine(uint32_t workers)
{
    if (workers == 0 || workers > 64)
        return -1;

    img_api_init_reset_engine();

    g_engine.worker_count = workers;
    g_engine.workers = g_workers;
    g_engine.scheduler = NULL;

    g_engine.caps = img_cpu_detect_caps();

    g_engine.global_pool = img_slab_create(
        512 * 1024 * 1024,
        32 * 1024 * 1024);

    if (!g_engine.global_pool)
    {
        img_api_init_reset_engine();
        return -1;
    }

    g_engine.user_data = img_template_registry_create();
    if (!g_engine.user_data)
    {
        img_api_init_reset_engine();
        return -1;
    }

    /* Ensure generated/committed pipelines are registered deterministically. */
    register_generated_pipelines();

    img_engine_init_compute(g_engine.caps);

    return 0;
}
