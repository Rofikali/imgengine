// ./include/imgengine/plugins/plugin_runner.h

#ifndef IMG_PLUGIN_RUNNER_H
#define IMG_PLUGIN_RUNNER_H

#include "imgengine/image.h"
#include "imgengine/context.h"
#include "imgengine/api.h"

int run_plugins(img_ctx_t *ctx,
                img_t *canvas,
                const img_job_t *job);

#endif