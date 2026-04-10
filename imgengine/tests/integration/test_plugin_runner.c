// ./tests/integration/test_plugin_runner.c
#include "plugins/plugin.h"

/**
 * run_pipeline - Iterates through registered plugins
 *
 * This follows the Linux VFS or Netfilter approach:
 * Each plugin decides if it wants to "hook" into the process.
 */
int run_pipeline(img_ctx_t *ctx, img_t *canvas, const img_job_t *job) {
    /* In a real kernel-style registry, this would be a linked list */
    extern img_plugin_t *plugin_registry[];

    for (int i = 0; plugin_registry[i] != NULL; i++) {
        img_plugin_t *p = plugin_registry[i];

        if (p->should_run(job)) {
            if (p->process(ctx, canvas, job) != 0) {
                // Plugin failed
                return -1;
            }
        }
    }
    return 0;
}
