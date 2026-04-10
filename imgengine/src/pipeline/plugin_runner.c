// ./src/pipeline/plugin_runner.c

#include "imgengine/plugins/plugin_runner.h"
#include "imgengine/plugins/plugin_registry.h"
#include <stdio.h>

int run_plugins(img_ctx_t *ctx, img_t *canvas, const img_job_t *job) {
    for (int i = 0; i < ctx->plugin_count; i++) {
        img_plugin_t *p = ctx->plugins[i];

        /* Speed: Check if we even need to run this (e.g. is bleed enabled?) */
        if (p->should_run && !p->should_run(job)) {
            continue;
        }

        if (!p->process(ctx, canvas, job)) {
            /* Log to stderr so Observability stack (Logstash) picks it up */
            fprintf(stderr, "[ENGINE] Plugin Failure: %s\n", p->name);
            return 0;
        }
    }
    return 1;
}
