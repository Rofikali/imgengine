// ./src/runtime/rpc_server.c
#include "runtime/rpc_server_internal.h"

void start_rpc_server(img_ctx_t *ctx) {
    int server_fd = img_rpc_server_open();
    if (server_fd < 0)
        return;

    for (;;) {
        int client = img_rpc_server_accept_client(server_fd);
        if (client < 0)
            continue;

        (void)img_rpc_server_process_client(ctx, client);
    }

    img_rpc_server_close(server_fd);
}
