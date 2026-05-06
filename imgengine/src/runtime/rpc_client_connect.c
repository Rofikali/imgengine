// ./src/runtime/rpc_client_connect.c
#include "runtime/rpc_client_internal.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 9090
#define SERVER_IP "127.0.0.1"

int img_rpc_client_connect(int sock) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    return connect(sock, (struct sockaddr *)&server, sizeof(server));
}
