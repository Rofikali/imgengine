// ./src/runtime/rpc_client_send_request.c
#include "runtime/rpc_client_internal.h"

#include <sys/socket.h>

int img_rpc_client_send_request(int sock, uint32_t op_code, img_buffer_t *buf) {
    img_rpc_request_t req;
    req.op_code = op_code;
    req.buffer.width = buf->width;
    req.buffer.height = buf->height;
    req.buffer.channels = buf->channels;
    req.buffer.stride = buf->stride;
    req.buffer.data_size = buf->height * buf->stride;

    if (send(sock, &req, sizeof(req), 0) < 0)
        return -1;
    if (send(sock, buf->data, req.buffer.data_size, 0) < 0)
        return -1;

    return 0;
}
