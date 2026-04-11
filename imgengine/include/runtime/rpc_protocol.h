// ./include/runtime/rpc_protocol.h

// include/runtime/rpc_protocol.h

#ifndef IMGENGINE_RPC_PROTOCOL_H
#define IMGENGINE_RPC_PROTOCOL_H

#include <stdint.h>

// 🔥 network-safe buffer descriptor
typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t stride;
    uint32_t data_size;
} img_rpc_buffer_t;

// 🔥 request packet
typedef struct
{
    uint32_t op_code;
    img_rpc_buffer_t buffer;
    // params follow
} img_rpc_request_t;

// 🔥 response packet
typedef struct
{
    uint32_t status;
    img_rpc_buffer_t buffer;
} img_rpc_response_t;

#endif