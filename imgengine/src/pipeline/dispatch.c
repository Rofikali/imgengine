// ./src/pipeline/dispatch.c

// src/pipeline/dispatch.c  (NEW FILE)

#define _GNU_SOURCE

#include "pipeline/dispatch.h"
#include "pipeline/jump_table.h"
#include <stddef.h>

img_dispatch_fn g_dispatch_fn = NULL;

void img_dispatch_register(img_dispatch_fn fn) { g_dispatch_fn = fn; }

/*
 * Default local dispatch — used when no router is registered.
 * Direct jump table lookup: O(1), no overhead.
 */
static int dispatch_local(img_ctx_t *ctx, uint32_t op_code, img_buffer_t *buf, void *params) {
    img_kernel_fn fn = g_jump_table[op_code];
    if (__builtin_expect(!fn, 0))
        return -1;
    fn(ctx, buf, params);
    return 0;
}

/*
 * img_dispatch()
 *
 * Called from pipeline/engine.c hot path.
 * Uses registered dispatcher (exec_router or local fallback).
 */
int img_dispatch(img_ctx_t *ctx, uint32_t op_code, img_buffer_t *buf, void *params) {
    if (__builtin_expect(g_dispatch_fn != NULL, 1))
        return g_dispatch_fn(ctx, op_code, buf, params);

    return dispatch_local(ctx, op_code, buf, params);
}
