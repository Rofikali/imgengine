// ./include/pipeline/dispatch.h
// include/pipeline/dispatch.h  (NEW FILE)

#ifndef IMGENGINE_PIPELINE_DISPATCH_H
#define IMGENGINE_PIPELINE_DISPATCH_H

#include <stdint.h>
#include "core/result.h"

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

/*
 * img_dispatch_fn — pluggable op dispatcher.
 *
 * Default: direct jump table call (local execution).
 * Alternative: exec_router (local or remote via RPC).
 *
 * Registered at startup. pipeline/engine.c calls through
 * this pointer — no direct dep on runtime/exec_router.
 */
typedef int (*img_dispatch_fn)(img_ctx_t *ctx, uint32_t op_code, img_buffer_t *buf, void *params);

extern img_dispatch_fn g_dispatch_fn;

void img_dispatch_register(img_dispatch_fn fn);

#endif /* IMGENGINE_PIPELINE_DISPATCH_H */
