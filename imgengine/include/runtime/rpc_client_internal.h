// ./include/runtime/rpc_client_internal.h
#ifndef IMGENGINE_RUNTIME_RPC_CLIENT_INTERNAL_H
#define IMGENGINE_RUNTIME_RPC_CLIENT_INTERNAL_H

#include "core/buffer.h"
#include "runtime/rpc_protocol.h"

int img_rpc_client_open(void);
int img_rpc_client_connect(int sock);
int img_rpc_client_send_request(int sock, uint32_t op_code, img_buffer_t *buf);
int img_rpc_client_recv_response(int sock, img_buffer_t *buf);
void img_rpc_client_close(int sock);

#endif /* IMGENGINE_RUNTIME_RPC_CLIENT_INTERNAL_H */
