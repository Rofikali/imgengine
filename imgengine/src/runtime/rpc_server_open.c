// ./src/runtime/rpc_server_open.c
#include "runtime/rpc_server_internal.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9090

int img_rpc_server_open(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return -1;

    struct sockaddr_in addr = {
        .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY};

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 16) != 0) {
        close(server_fd);
        return -1;
    }

    return server_fd;
}
