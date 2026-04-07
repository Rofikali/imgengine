// src/runtime/rpc_server.c

#include "runtime/rpc_protocol.h"
#include "pipeline/jump_table.h"
#include "api/v1/img_buffer_utils.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 9090

void start_rpc_server(img_ctx_t *ctx)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY};

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 16);

    while (1)
    {
        int client = accept(server_fd, NULL, NULL);

        img_rpc_request_t req;
        recv(client, &req, sizeof(req), 0);

        // allocate buffer
        uint8_t *data = malloc(req.buffer.data_size);

        recv(client, data, req.buffer.data_size, 0);

        img_buffer_t buf = {
            .width = req.buffer.width,
            .height = req.buffer.height,
            .channels = req.buffer.channels,
            .stride = req.buffer.stride,
            .data = data};

        // 🔥 execute
        g_jump_table[req.op_code](ctx, &buf, NULL);

        // send response
        img_rpc_response_t res = {
            .status = 0,
            .buffer = req.buffer};

        send(client, &res, sizeof(res), 0);
        send(client, buf.data, req.buffer.data_size, 0);

        free(data);
        close(client);
    }
}