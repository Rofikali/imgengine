
// include/runtime/exec_router.h

#ifndef IMGENGINE_EXEC_ROUTER_H
#define IMGENGINE_EXEC_ROUTER_H

#include <stdint.h>

// Forward declarations
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

// Execution modes
typedef enum
{
    IMG_EXEC_LOCAL = 0,
    IMG_EXEC_REMOTE = 1
} img_exec_mode_t;

// Route decision
img_exec_mode_t img_exec_route(uint32_t op_code);

// Execute (local or remote)
int img_exec_dispatch(
    img_ctx_t *ctx,
    uint32_t op_code,
    img_buffer_t *buf,
    void *params);

#endif