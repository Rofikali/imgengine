// ./src/runtime/rpc_client_recv_response.c
#include "runtime/rpc_client_internal.h"

#include <sys/socket.h>

int img_rpc_client_recv_response(int sock, img_buffer_t *buf) {
    img_rpc_response_t res;
    if (recv(sock, &res, sizeof(res), 0) < 0)
        return -1;
    if (recv(sock, buf->data, res.buffer.data_size, 0) < 0)
        return -1;

    return 0;
}
