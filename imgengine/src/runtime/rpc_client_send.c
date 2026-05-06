// ./src/runtime/rpc_client_send.c
#include "runtime/rpc_client.h"
#include "runtime/rpc_client_internal.h"

int img_rpc_send(uint32_t op_code, img_buffer_t *buf, void *params) {
    (void)params;

    int sock = img_rpc_client_open();
    if (sock < 0)
        return -1;

    if (img_rpc_client_connect(sock) < 0) {
        img_rpc_client_close(sock);
        return -1;
    }

    if (img_rpc_client_send_request(sock, op_code, buf) < 0) {
        img_rpc_client_close(sock);
        return -1;
    }

    if (img_rpc_client_recv_response(sock, buf) < 0) {
        img_rpc_client_close(sock);
        return -1;
    }

    img_rpc_client_close(sock);
    return 0;
}
