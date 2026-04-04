// src/core/init.c
#include "core/context_internal.h"
#include "arch/cpu_caps.h"
#include "pipeline/jump_table.h"
#include "runtime/plugin_loader.h"

// forward
void img_plugins_init_all(void);
void img_hw_register_kernels(cpu_caps_t caps);

void img_engine_init_plugins()
{
    img_plugin_load_all("./plugins_build");
}

void img_engine_init(img_engine_t *engine)
{
    if (!engine)
        return;

    // ================= 1. DETECT CPU =================
    engine->caps = img_cpu_detect_caps();

    // ================= 2. INIT JUMP TABLE =================
    img_jump_table_init(engine->caps);

    // ================= 3. REGISTER PLUGINS =================
    img_plugins_init_all();

    // ================= 4. OPTIONAL SIMD OVERRIDE =================
    img_hw_register_kernels(engine->caps);
}
