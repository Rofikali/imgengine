// // src/api/api.c

#include "api/v1/img_api.h"
#include "include/core/context_internal.h"
#include "memory/slab.h"
#include "plugins/plugin_internal.h"

#include <stdlib.h>

// ==========================================
// INIT
// ==========================================
img_engine_t *img_api_init(uint32_t workers)
{
    img_engine_t *engine = malloc(sizeof(img_engine_t));
    if (!engine)
        return NULL;

    engine->worker_count = workers;

    engine->global_pool = img_slab_create(
        64 * 1024 * 1024, // 64MB
        64 * 1024         // 64KB blocks
    );

    img_plugins_init_all();

    return engine;
}

// ==========================================
// FAST PROCESS (Minimal Working Path)
// ==========================================
int img_api_process_fast(
    img_engine_t *engine,
    uint8_t *input,
    size_t input_size,
    uint8_t **output,
    size_t *output_size)
{
    (void)engine;
    (void)input;
    (void)input_size;

    // 🔥 Placeholder pipeline (you will wire real later)
    *output = input;
    *output_size = input_size;

    return 0;
}

// ==========================================
// SHUTDOWN
// ==========================================
void img_api_shutdown(img_engine_t *engine)
{
    if (!engine)
        return;

    img_slab_destroy(engine->global_pool);

    free(engine);
}
