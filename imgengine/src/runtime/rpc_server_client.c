// ./src/runtime/rpc_server_client.c
#include "runtime/rpc_server_internal.h"
#include "pipeline/jump_table.h"
#include "core/buffer.h"

#include <stdlib.h>
#include <unistd.h>

int img_rpc_server_process_client(img_ctx_t *ctx, int client) {
    if (!ctx || client < 0)
        return -1;

    img_rpc_request_t req;
    if (img_rpc_server_recv_all(client, &req, sizeof(req)) != 0) {
        close(client);
        return -1;
    }

    if (req.buffer.data_size == 0 || req.buffer.data_size > (64u * 1024u * 1024u)) {
        close(client);
        return -1;
    }

    uint8_t *data = malloc(req.buffer.data_size);
    if (!data) {
        close(client);
        return -1;
    }

    if (img_rpc_server_recv_all(client, data, req.buffer.data_size) != 0) {
        free(data);
        close(client);
        return -1;
    }

    img_buffer_t buf = {.width = req.buffer.width,
                        .height = req.buffer.height,
                        .channels = req.buffer.channels,
                        .stride = req.buffer.stride,
                        .data = data};

    if (req.op_code >= IMG_MAX_OPS || !g_jump_table[req.op_code]) {
        free(data);
        close(client);
        return -1;
    }

    g_jump_table[req.op_code](ctx, &buf, NULL);

    img_rpc_response_t res = {.status = 0, .buffer = req.buffer};

    int rc = 0;
    if (img_rpc_server_send_all(client, &res, sizeof(res)) != 0)
        rc = -1;
    if (rc == 0 && img_rpc_server_send_all(client, buf.data, req.buffer.data_size) != 0)
        rc = -1;

    free(data);
    close(client);
    return rc;
}
