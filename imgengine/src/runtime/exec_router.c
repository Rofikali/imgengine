
// ./src/runtime/exec_router.c

#include "runtime/exec_router.h"
#include "runtime/rpc_client.h"
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

// 🔥 smart routing (L8 brain)
img_exec_mode_t img_exec_route(uint32_t op_code)
{
    switch (op_code)
    {
    case OP_RESIZE:
        return IMG_EXEC_LOCAL;

    default:
        return IMG_EXEC_REMOTE;
    }
}

int img_exec_dispatch(
    img_ctx_t *ctx,
    uint32_t op_code,
    img_buffer_t *buf,
    void *params)
{
    img_exec_mode_t mode = img_exec_route(op_code);

    if (mode == IMG_EXEC_LOCAL)
    {
        g_jump_table[op_code](ctx, buf, params);
        return 0;
    }
    else
    {
        return img_rpc_send(op_code, buf, params);
    }
}