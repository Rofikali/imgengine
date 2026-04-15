// src/startup/engine_init.c  (NEW FILE)

// ================================================================
// STEP 6 OF 6: Fix core/ depending upward on everything
// core/init.c → arch/, pipeline/, runtime/, security/ (wrong)
// Fix: move init.c logic to src/startup/engine_init.c
// core/ becomes pure types + lifecycle — no upward deps
// ================================================================

// Move content of src/core/init.c here.
// startup/ is above runtime/ in the DAG — it can include everything.

#define _GNU_SOURCE

/*
 * engine_init.c — top-level engine initialization.
 *
 * Moved from core/init.c because core/ (index 0) must not depend
 * on arch/, pipeline/, runtime/, or security/.
 *
 * startup/ sits above all layers — it orchestrates them.
 * It is NOT in the imgengine library layer DAG; it is the entry
 * point that wires everything together.
 */

#include "core/context_internal.h"
#include "arch/cpu_caps.h"
#include "pipeline/jump_table.h"
#include "runtime/plugin_loader.h"
#include "security/sandbox.h"
#include "api/io_vtable.h"
#include <stdlib.h>

/* declared in pipeline/hardware_registry.c */
void img_hw_register_kernels(cpu_caps_t caps);

/* declared in src/plugins/plugin_registry.c */
void img_plugins_init_all(void);

/* declared in src/io/io_register.c */
void img_io_register_defaults(void);

/* declared in src/pipeline/fused_registry.c */
void img_fused_init(cpu_caps_t caps);

void img_engine_init(img_engine_t *engine)
{
    if (!engine)
        return;

    /* SECURITY FIRST — sandbox before any untrusted input */
    if (!img_security_enter_sandbox())
        abort();

    /* CPU capability detection */
    engine->caps = img_cpu_detect_caps();

    /* Jump table — must be before plugins (plugins register into it) */
    img_jump_table_init(engine->caps);

    /* SIMD kernel registration — CPU-optimal implementations */
    img_hw_register_kernels(engine->caps);

    /* Fused batch kernels */
    img_fused_init(engine->caps);

    /* Static plugins (ELF section registration) */
    img_plugins_init_all();

    /* Dynamic plugins (dlopen from plugins_build/) */
    img_plugin_load_all("./plugins_build");

    /* I/O vtable — wires decoder/encoder into api/ */
    img_io_register_defaults();
}
