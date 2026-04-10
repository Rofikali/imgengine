
// // src/core/init.c

#include "core/context_internal.h"
#include "arch/cpu_caps.h"
#include "pipeline/jump_table.h"
#include "runtime/plugin_loader.h"
#include "security/sandbox.h"

#include <stdlib.h>

void img_plugins_init_all(void);
void img_hw_register_kernels(cpu_caps_t caps);

void img_engine_init(img_engine_t *engine)
{
    if (!engine)
        return;

    // 🔥 SECURITY FIRST
    if (!img_security_enter_sandbox())
        abort();

    // 🔥 CPU DETECT
    engine->caps = img_cpu_detect_caps();

    // 🔥 DISPATCH
    img_jump_table_init(engine->caps);

    // 🔥 PLUGINS
    img_plugins_init_all();
    img_plugin_load_all("./plugins_build");

    // 🔥 SIMD
    img_hw_register_kernels(engine->caps);
}
