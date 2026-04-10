
// include/runtime/rpc_client.h

#ifndef IMGENGINE_RPC_CLIENT_H
#define IMGENGINE_RPC_CLIENT_H

#include <stdint.h>

// Forward declarations
typedef struct img_buffer img_buffer_t;

int img_rpc_send(
    uint32_t op_code,
    img_buffer_t *buf,
    void *params);

#endif