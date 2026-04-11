// ./src/runtime/rpc_client.c

// ./src/runtime/rpc_client.c


#include "runtime/rpc_client.h"
#include "runtime/rpc_protocol.h"
// #include "api/v1/img_types.h"
// #include "pipeline/pipeline_types.h"
#include "core/buffer.h"

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 9090
#define SERVER_IP "127.0.0.1"

int img_rpc_send(uint32_t op_code, img_buffer_t *buf, void *params)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        return -1;

    // Build request
    img_rpc_request_t req;
    req.op_code = op_code;
    req.buffer.width = buf->width;
    req.buffer.height = buf->height;
    req.buffer.channels = buf->channels;
    req.buffer.stride = buf->stride;
    req.buffer.data_size = buf->height * buf->stride;

    // Send header
    send(sock, &req, sizeof(req), 0);

    // Send raw image
    send(sock, buf->data, req.buffer.data_size, 0);

    // Receive response header
    img_rpc_response_t res;
    recv(sock, &res, sizeof(res), 0);

    // Receive output data (overwrite buffer)
    recv(sock, buf->data, res.buffer.data_size, 0);

    close(sock);
    return 0;
}
