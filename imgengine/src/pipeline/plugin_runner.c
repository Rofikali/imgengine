// // src/pipeline/plugin_runner.c

// #include <stdio.h>
// #include "imgengine/plugins/plugin_registry.h"

// int run_plugins(img_ctx_t *ctx,
//                 img_t *canvas,
//                 const img_job_t *job)
// {
//     int count = 0;
//     img_plugin_t **plugins = plugin_get_all(&count);

//     for (int i = 0; i < count; i++)
//     {
//         img_plugin_t *p = plugins[i];

//         // skip disabled plugins
//         if (!p)
//             continue;

//         if (!p->process(ctx, canvas, job))
//         {
//             printf("❌ Plugin failed: %s\n", p->name);
//             return 0;
//         }
//     }

//     return 1;
// }
#include "imgengine/plugins/plugin_runner.h"
#include "imgengine/plugins/plugin_registry.h"
#include <stdio.h>

int run_plugins(img_ctx_t *ctx, img_t *canvas, const img_job_t *job)
{
    for (int i = 0; i < ctx->plugin_count; i++)
    {
        img_plugin_t *p = ctx->plugins[i];

        /* Speed: Check if we even need to run this (e.g. is bleed enabled?) */
        if (p->should_run && !p->should_run(job))
        {
            continue;
        }

        if (!p->process(ctx, canvas, job))
        {
            /* Log to stderr so Observability stack (Logstash) picks it up */
            fprintf(stderr, "[ENGINE] Plugin Failure: %s\n", p->name);
            return 0;
        }
    }
    return 1;
}
