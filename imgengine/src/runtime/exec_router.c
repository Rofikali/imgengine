// ./src/runtime/exec_router.c
// src/runtime/exec_router.c

// ================================================================
// FILE 3: src/runtime/exec_router.c
// Wire exec_router into the pipeline hot path.
// Currently: jump table calls are direct (local always).
// With exec_router: each op_code is routed — local SIMD or remote RPC.
// This enables the distributed execution model from RFC Section 3.
// ================================================================

#include "runtime/exec_router.h"
#include "runtime/rpc_client.h"
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

/*
 * img_exec_route()
 *
 * Routing policy — determines whether an op runs locally or remotely.
 *
 * Current policy (v1):
 *   OP_RESIZE   → LOCAL  (AVX2 SIMD, fastest on local CPU)
 *   OP_GRAYSCALE → LOCAL (fused with resize in hot path)
 *   OP_BRIGHTNESS → LOCAL (fused kernel)
 *   everything else → REMOTE (offload to worker node)
 *
 * Future: policy loaded from config, adaptive based on load metrics.
 */
img_exec_mode_t img_exec_route(uint32_t op_code) {
    switch (op_code) {
    case OP_RESIZE:
    case OP_RESIZE_H:
    case OP_RESIZE_V:
    case OP_GRAYSCALE:
    case OP_BRIGHTNESS:
        return IMG_EXEC_LOCAL;

    default:
        return IMG_EXEC_REMOTE;
    }
}

/*
 * img_exec_dispatch()
 *
 * Execute one op — locally via jump table or remotely via RPC.
 *
 * HOT PATH (local): direct jump table call, zero overhead vs current code.
 * COLD PATH (remote): RPC send — network latency, used for heavy ops.
 *
 * Called from pipeline/engine.c in the execution loop.
 * Replace direct g_jump_table[op_code](ctx, buf, params) calls with
 * img_exec_dispatch(ctx, op_code, buf, params) to enable routing.
 */
int img_exec_dispatch(img_ctx_t *ctx, uint32_t op_code, img_buffer_t *buf, void *params) {
    img_exec_mode_t mode = img_exec_route(op_code);

    if (mode == IMG_EXEC_LOCAL) {
        img_kernel_fn fn = g_jump_table[op_code];
        if (!fn)
            return -1; /* op not registered */
        fn(ctx, buf, params);
        return 0;
    }

    /* REMOTE: serialize and send via RPC */
    return img_rpc_send(op_code, buf, params);
}
