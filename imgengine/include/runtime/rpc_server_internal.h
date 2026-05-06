// ./include/runtime/rpc_server_internal.h
#ifndef IMGENGINE_RUNTIME_RPC_SERVER_INTERNAL_H
#define IMGENGINE_RUNTIME_RPC_SERVER_INTERNAL_H

#include "core/context_internal.h"
#include "runtime/rpc_protocol.h"

#include <stddef.h>

int img_rpc_server_open(void);
void img_rpc_server_close(int server_fd);
int img_rpc_server_accept_client(int server_fd);
int img_rpc_server_recv_all(int fd, void *buf, size_t size);
int img_rpc_server_send_all(int fd, const void *buf, size_t size);
int img_rpc_server_process_client(img_ctx_t *ctx, int client);

#endif /* IMGENGINE_RUNTIME_RPC_SERVER_INTERNAL_H */
